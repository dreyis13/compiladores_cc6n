/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "lex.h"
#include "diag.h"
#include "symtab.h"
#include "codegen.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Variáveis estáticas do parser */
static Token current;               // token atual
static const char *src_base;        // nome base do arquivo (para logs)
static int tokens_flag = 0;         // flag --tokens
static int trace_flag = 0;          // flag --trace
static int inside_function = 0;     // indica se está dentro de uma função (para retorno)

/* Offsets para geração de código (posição das variáveis na pilha) */
static int global_offset = 0;
static int local_offset = 0;

/* Protótipos das funções internas */
static void advance(void);
static void match(TokenType expected);
static DataType parse_type(void);
static void parse_declarations(int is_global, int *offset_counter);
static void parse_params(int *param_count);
static void parse_globals(void);
static void parse_subs(void);
static void parse_proc(void);
static void parse_fn(void);
static void parse_block(void);
static void parse_command(void);
static void parse_print(void);
static void parse_scan(void);
static void parse_if(void);
static void parse_match(void);
static void parse_for(void);
static void parse_loop_while(void);
static void parse_loop_until(void);
static void parse_ret(void);
static DataType parse_expr(void);
static DataType parse_logical_and(void);
static DataType parse_relational(void);
static DataType parse_simple_expr(void);
static DataType parse_term(void);
static DataType parse_factor(void);

/* Função de rótulo (definida em codegen.c) */
char* new_label(void);

/* Avança para o próximo token, registrando se necessário (para --tokens) */
static void advance(void) {
    if (tokens_flag && current.type != sEOF && current.type != sERROR) {
        log_write_token(current);
    }
    free(current.lexeme);
    current = lex_next();
}

/* Verifica se o token atual é o esperado; se não, reporta erro e tenta recuperar */
static void match(TokenType expected) {
    if (current.type == expected) {
        advance();
    } else {
        diag_error(current.line, token_type_name(expected), token_type_name(current.type));
        /* Recuperação: avança até encontrar ponto e vírgula, end, start ou else */
        while (current.type != sPONTOVIRG && current.type != sEND &&
               current.type != sEOF && current.type != sSTART && current.type != sELSE)
            advance();
        if (current.type == sPONTOVIRG || current.type == sEND)
            advance();
    }
}

/* Lê um tipo básico (int, bool, char) e retorna sua representação interna */
static DataType parse_type(void) {
    diag_info("Entrando em parse_type");
    DataType t;
    if (current.type == sINT) { t = TIPO_INT; advance(); }
    else if (current.type == sBOOL) { t = TIPO_BOOL; advance(); }
    else if (current.type == sCHAR) { t = TIPO_CHAR; advance(); }
    else {
        diag_error(current.line, "tipo (int, bool, char)", token_type_name(current.type));
        t = TIPO_INT;
    }
    diag_info("Saindo de parse_type");
    return t;
}

/* Declaração de variáveis (globais ou locais). Suporta vetores e lista de identificadores */
static void parse_declarations(int is_global, int *offset_counter) {
    diag_info("Entrando em parse_declarations");
    do {
        char *ids[100];
        int id_count = 0;
        int is_vector = 0;
        int vec_size = 0;

        /* Primeiro identificador */
        ids[id_count++] = strdup(current.lexeme);
        match(sIDENTIF);

        /* Verifica se é vetor: [ tamanho ] */
        if (current.type == sABRECOL) {
            is_vector = 1;
            advance();
            if (current.type == sCTEINT) {
                vec_size = atoi(current.lexeme);
                advance();
            } else {
                diag_error(current.line, "constante inteira", token_type_name(current.type));
            }
            match(sFECHACOL);
        }

        /* Possíveis outros identificadores separados por vírgula */
        while (current.type == sVIRG) {
            advance();
            if (current.type == sIDENTIF) {
                ids[id_count++] = strdup(current.lexeme);
                advance();
                /* Se for vetor, assume mesmo tamanho para todos */
                if (current.type == sABRECOL) {
                    advance();
                    if (current.type == sCTEINT) advance();
                    else diag_error(current.line, "constante inteira", token_type_name(current.type));
                    match(sFECHACOL);
                }
            } else {
                diag_error(current.line, "identificador", token_type_name(current.type));
            }
        }

        /* Dois pontos e tipo */
        match(sDOISPONTOS);
        DataType base_type = parse_type();
        match(sPONTOVIRG);

        /* Insere cada identificador na tabela de símbolos e atualiza offset */
        for (int i = 0; i < id_count; i++) {
            if (is_vector) {
                symtab_insert(ids[i], CAT_VECTOR, base_type, vec_size);
                symtab_set_offset(ids[i], *offset_counter);
                *offset_counter += vec_size + 1;   // célula extra para tamanho
            } else {
                symtab_insert(ids[i], CAT_VAR, base_type, 0);
                symtab_set_offset(ids[i], *offset_counter);
                (*offset_counter)++;
            }
            free(ids[i]);
        }
    } while (current.type == sIDENTIF);
    diag_info("Saindo de parse_declarations");
}

/* Parâmetros de sub-rotina (apenas variáveis simples) */
static void parse_params(int *param_count) {
    diag_info("Entrando em parse_params");
    do {
        char *pname = strdup(current.lexeme);
        match(sIDENTIF);
        match(sDOISPONTOS);
        DataType t = parse_type();
        symtab_insert(pname, PARAM, t, 0);
        symtab_set_offset(pname, 4 + (*param_count) * 4);  // offset a partir do frame pointer
        (*param_count)++;
        free(pname);
        if (current.type == sVIRG) advance();
        else break;
    } while (current.type == sIDENTIF);
    diag_info("Saindo de parse_params");
}

/* Seção globals */
static void parse_globals(void) {
    diag_info("Entrando em parse_globals");
    match(sGLOBALS);
    parse_declarations(1, &global_offset);
    /* Inicializa vetores globais: armazena o tamanho na célula base */
    Scope *sc = symtab_get_current_scope();
    for (Symbol *s = sc->symbols; s; s = s->next) {
        if (s->category == CAT_VECTOR) {
            emit_crct(s->extra);
            emit_armz(s->offset);
        }
    }
    diag_info("Saindo de parse_globals");
}

/* Procedimento */
static void parse_proc(void) {
    diag_info("Entrando em parse_proc");
    match(sPROC);
    char proc_name[256];
    strcpy(proc_name, current.lexeme);
    match(sIDENTIF);
    symtab_insert(proc_name, PROC, TIPO_INT, 0);
    char label[256];
    sprintf(label, "_%s", proc_name);
    symtab_set_label(proc_name, label);
    emit_label(label);                       // rótulo para chamada

    char desc[256];
    sprintf(desc, "proc:%s.locals", proc_name);
    symtab_enter_scope(desc);
    local_offset = 0;
    int param_count = 0;
    match(sABREPAR);
    if (current.type != sFECHAPAR) parse_params(&param_count);
    match(sFECHAPAR);
    symtab_update_extra(proc_name, param_count);

    if (current.type == sLOCALS) {
        advance();
        parse_declarations(0, &local_offset);
    }
    symtab_set_total_locals(local_offset);
    emit_amem(local_offset);                 // aloca espaço para variáveis locais

    /* Inicializa vetores locais com seus tamanhos */
    Scope *sc = symtab_get_current_scope();
    for (Symbol *s = sc->symbols; s; s = s->next) {
        if (s->category == CAT_VECTOR) {
            emit_crct(s->extra);
            emit_armz(s->offset);
        }
    }
    inside_function = 0;
    parse_block();                           // corpo do procedimento
    emit_dmem(local_offset);
    emit_retorno();
    symtab_exit_scope();
    diag_info("Saindo de parse_proc");
}

/* Função (semelhante a procedimento, mas com tipo de retorno) */
static void parse_fn(void) {
    diag_info("Entrando em parse_fn");
    match(sFN);
    char fn_name[256];
    strcpy(fn_name, current.lexeme);
    match(sIDENTIF);
    symtab_insert(fn_name, FUNC, TIPO_INT, 0);
    char label[256];
    sprintf(label, "_%s", fn_name);
    symtab_set_label(fn_name, label);
    emit_label(label);

    char desc[256];
    sprintf(desc, "fn:%s.locals", fn_name);
    symtab_enter_scope(desc);
    local_offset = 0;
    int param_count = 0;
    match(sABREPAR);
    if (current.type != sFECHAPAR) parse_params(&param_count);
    match(sFECHAPAR);
    symtab_update_extra(fn_name, param_count);
    match(sDOISPONTOS);
    DataType ret_type = parse_type(); (void)ret_type;

    if (current.type == sLOCALS) {
        advance();
        parse_declarations(0, &local_offset);
    }
    symtab_set_total_locals(local_offset);
    emit_amem(local_offset);

    Scope *sc = symtab_get_current_scope();
    for (Symbol *s = sc->symbols; s; s = s->next) {
        if (s->category == CAT_VECTOR) {
            emit_crct(s->extra);
            emit_armz(s->offset);
        }
    }
    inside_function = 1;
    parse_block();
    emit_dmem(local_offset);
    emit_retorno();
    symtab_exit_scope();
    diag_info("Saindo de parse_fn");
}

/* Lista de sub-rotinas (procedimentos e funções) */
static void parse_subs(void) {
    diag_info("Entrando em parse_subs");
    while (current.type == sPROC || current.type == sFN) {
        if (current.type == sPROC) parse_proc();
        else parse_fn();
    }
    diag_info("Saindo de parse_subs");
}

/* Comando print: suporta strings e expressões numéricas */
static void parse_print(void) {
    diag_info("Entrando em parse_print");
    match(sPRINT);
    match(sABREPAR);
    do {
        if (current.type == sSTRING) {
            char *str = current.lexeme;
            for (int i = 0; str[i] != '\0'; i++) {
                emit_crct(str[i]);
                emit_chpr();           // imprime caractere
            }
            advance();
        } else {
            parse_expr();
            emit_chpd();               // imprime número decimal
        }
        if (current.type == sVIRG) advance();
        else break;
    } while (1);
    match(sFECHAPAR);
    match(sPONTOVIRG);
    diag_info("Saindo de parse_print");
}

/* Comando scan: lê valor do teclado e armazena em variável ou posição de vetor */
static void parse_scan(void) {
    diag_info("Entrando em parse_scan");
    match(sSCAN);
    match(sABREPAR);
    if (current.type == sIDENTIF) {
        char *var = strdup(current.lexeme);
        advance();
        int offset = symtab_get_offset(var);
        if (offset == -1) {
            diag_error(current.line, "variável declarada", var);
        }
        if (current.type == sABRECOL) {
            /* Leitura para elemento de vetor: v[índice] */
            advance();
            parse_expr();               // índice
            match(sFECHACOL);
            emit_crct(offset);
            emit_soma();                // endereço = base + índice
            emit_chle();                // lê valor e empilha
            emit_armi();                // armazena no endereço calculado
        } else {
            emit_chle();
            emit_armz(offset);
        }
        free(var);
    } else {
        diag_error(current.line, "identificador", token_type_name(current.type));
    }
    match(sFECHAPAR);
    match(sPONTOVIRG);
    diag_info("Saindo de parse_scan");
}

/* Comando if com else opcional */
static void parse_if(void) {
    diag_info("Entrando em parse_if");
    match(sIF);
    match(sABREPAR);
    parse_expr();
    char *label_false = new_label();
    char *label_end = new_label();
    emit_dsvf(label_false);        // desvia se condição falsa
    parse_command();               // bloco then
    emit_dsvs(label_end);          // pula o else
    emit_label(label_false);
    if (current.type == sELSE) {
        advance();
        parse_command();           // bloco else
    }
    emit_label(label_end);
    free(label_false);
    free(label_end);
    diag_info("Saindo de parse_if");
}

/* Aloca um espaço temporário na pilha (para match, por exemplo) */
static int allocate_temp(void) {
    int temp = local_offset;
    local_offset++;
    symtab_set_total_locals(local_offset);
    return temp;
}

/* Comando match (seleção múltipla) com suporte a valores únicos e intervalos */
static void parse_match(void) {
    diag_info("Entrando em parse_match");
    match(sMATCH);
    match(sABREPAR);
    int temp = allocate_temp();          // variável temporária para guardar a expressão
    parse_expr();
    emit_armz(temp);
    match(sFECHAPAR);

    char *label_end = new_label();

    while (current.type == sWHEN) {
        advance();
        char *label_next = new_label();

        /* Processa um valor ou intervalo (ignora listas completas por simplicidade) */
        if (current.type == sCTEINT) {
            int val = atoi(current.lexeme);
            advance();
            if (current.type == sPTOPTO) {
                /* Intervalo val .. val2 */
                advance();
                int val2 = atoi(current.lexeme);
                advance();
                emit_crvl(temp);
                emit_crct(val);
                emit_cmmeq();            // >= val
                emit_crvl(temp);
                emit_crct(val2);
                emit_cmmaq();            // <= val2
                emit_conj();             // combina as duas condições
            } else {
                /* Valor único */
                emit_crvl(temp);
                emit_crct(val);
                emit_cmeq();
            }
            /* Se houver vírgula (lista), avança ignorando (trata apenas o primeiro) */
            while (current.type == sVIRG) {
                advance();
                if (current.type == sCTEINT) advance();
                if (current.type == sPTOPTO) { advance(); advance(); }
            }
        } else {
            diag_error(current.line, "constante inteira", token_type_name(current.type));
            advance();
        }

        emit_dsvf(label_next);          // se não corresponde, vai para próximo when
        match(sIMPLIC);
        parse_command();
        emit_dsvs(label_end);           // após executar, sai do match
        emit_label(label_next);
        free(label_next);
    }

    if (current.type == sOTHERWISE) {
        advance();
        match(sIMPLIC);
        parse_command();
    }
    match(sEND);
    emit_label(label_end);
    free(label_end);
    diag_info("Saindo de parse_match");
}

/* Comando for (incremento unitário, sem step) */
static void parse_for(void) {
    diag_info("Entrando em parse_for");
    match(sFOR);
    char *ctrl = strdup(current.lexeme);
    match(sIDENTIF);
    if (symtab_lookup(ctrl) == NULL)
        diag_error(current.line, "variável declarada", ctrl);
    int offset = symtab_get_offset(ctrl);
    match(sATRIB);
    parse_expr();
    emit_armz(offset);                  // inicializa variável de controle
    match(sTO);
    parse_expr();                       // limite
    char *label_start = new_label();
    char *label_exit = new_label();
    emit_label(label_start);
    emit_crvl(offset);
    emit_cmma();                        // controle > limite ?
    emit_dsvf(label_exit);
    if (current.type == sDO) advance();
    parse_command();
    emit_crvl(offset);
    emit_crct(1);
    emit_soma();
    emit_armz(offset);                  // incrementa
    emit_dsvs(label_start);
    emit_label(label_exit);
    free(ctrl);
    free(label_start);
    free(label_exit);
    diag_info("Saindo de parse_for");
}

/* Loop while: repita enquanto condição verdadeira */
static void parse_loop_while(void) {
    diag_info("Entrando em parse_loop_while");
    match(sLOOP);
    match(sWHILE);
    match(sABREPAR);
    char *label_start = new_label();
    char *label_end = new_label();
    emit_label(label_start);
    parse_expr();
    emit_dsvf(label_end);
    parse_command();
    emit_dsvs(label_start);
    emit_label(label_end);
    free(label_start);
    free(label_end);
    diag_info("Saindo de parse_loop_while");
}

/* Loop until: repita até que condição se torne verdadeira */
static void parse_loop_until(void) {
    diag_info("Entrando em parse_loop_until");
    match(sLOOP);
    char *label_start = new_label();
    emit_label(label_start);
    parse_command();
    match(sUNTIL);
    match(sABREPAR);
    parse_expr();
    match(sFECHAPAR);
    match(sPONTOVIRG);
    emit_dsvf(label_start);             // repete enquanto condição falsa
    free(label_start);
    diag_info("Saindo de parse_loop_until");
}

/* Comando return (retorna valor de função) */
static void parse_ret(void) {
    diag_info("Entrando em parse_ret");
    match(sRET);
    parse_expr();
    match(sPONTOVIRG);
    emit_dmem(symtab_get_total_locals());
    emit_retorno();
    diag_info("Saindo de parse_ret");
}

/* Bloco de comandos delimitado por start/end */
static void parse_block(void) {
    diag_info("Entrando em parse_block");
    match(sSTART);
    while (current.type != sEND && current.type != sEOF) {
        parse_command();
    }
    match(sEND);
    diag_info("Saindo de parse_block");
}

/* Comando genérico: despacha para a função adequada */
static void parse_command(void) {
    diag_info("Entrando em parse_command");
    if (current.type == sSTART) {
        parse_block();
    } else if (current.type == sPRINT) {
        parse_print();
    } else if (current.type == sSCAN) {
        parse_scan();
    } else if (current.type == sIF) {
        parse_if();
    } else if (current.type == sMATCH) {
        parse_match();
    } else if (current.type == sFOR) {
        parse_for();
    } else if (current.type == sLOOP) {
        Token save = current;
        advance();
        if (current.type == sWHILE) {
            current = save;
            parse_loop_while();
        } else {
            current = save;
            parse_loop_until();
        }
    } else if (current.type == sRET) {
        parse_ret();
    } else if (current.type == sIDENTIF) {
        char *id = strdup(current.lexeme);
        advance();
        if (current.type == sATRIB) {
            /* Atribuição simples: id := expr */
            advance();
            parse_expr();
            match(sPONTOVIRG);
            int offset = symtab_get_offset(id);
            if (offset == -1) diag_error(current.line, "variável declarada", id);
            else emit_armz(offset);
        } else if (current.type == sABREPAR) {
            /* Chamada de procedimento/função */
            advance();
            int arg_count = 0;
            while (current.type != sFECHAPAR && current.type != sEOF) {
                parse_expr();
                arg_count++;
                if (current.type == sVIRG) advance();
                else break;
            }
            match(sFECHAPAR);
            match(sPONTOVIRG);
            const char *label = symtab_get_label(id);
            if (label) {
                emit_para(arg_count);
                emit_cham(label);
            } else {
                diag_error(current.line, "procedimento/função declarado", id);
            }
        } else if (current.type == sABRECOL) {
            /* Atribuição a vetor: v[expr] := expr */
            advance();
            parse_expr();               // índice
            match(sFECHACOL);
            match(sATRIB);
            parse_expr();               // valor RHS
            emit_troca();               // troca RHS e índice
            int base_offset = symtab_get_offset(id);
            if (base_offset == -1) {
                diag_error(current.line, "vetor declarado", id);
            } else {
                emit_crct(base_offset);
                emit_soma();            // endereço = base + índice
                emit_troca();           // (RHS, endereço)
                emit_armi();            // armazena RHS no endereço
            }
            match(sPONTOVIRG);
        } else {
            diag_error(current.line, ":=", token_type_name(current.type));
        }
        free(id);
    } else {
        diag_error(current.line, "comando", token_type_name(current.type));
        advance();
    }
    diag_info("Saindo de parse_command");
}

/* Fator de expressão: constante, variável, chamada de função, acesso a vetor, parênteses */
static DataType parse_factor(void) {
    diag_info("Entrando em parse_factor");
    DataType type = TIPO_INT;
    if (current.type == sCTEINT) {
        int val = atoi(current.lexeme);
        emit_crct(val);
        advance();
        type = TIPO_INT;
    } else if (current.type == sCTECHAR) {
        int val = current.lexeme[0];
        emit_crct(val);
        advance();
        type = TIPO_CHAR;
    } else if (current.type == sTRUE) {
        emit_crct(1);
        advance();
        type = TIPO_BOOL;
    } else if (current.type == sFALSE) {
        emit_crct(0);
        advance();
        type = TIPO_BOOL;
    } else if (current.type == sIDENTIF) {
        char *id = strdup(current.lexeme);
        advance();
        if (current.type == sABREPAR) {
            /* Chamada de função */
            advance();
            int arg_count = 0;
            while (current.type != sFECHAPAR && current.type != sEOF) {
                parse_expr();
                arg_count++;
                if (current.type == sVIRG) advance();
                else break;
            }
            match(sFECHAPAR);
            const char *label = symtab_get_label(id);
            if (label) {
                emit_para(arg_count);
                emit_cham(label);
                type = TIPO_INT;
            } else {
                diag_error(current.line, "função declarada", id);
            }
        } else if (current.type == sABRECOL) {
            /* Acesso a vetor: v[expr] */
            advance();
            parse_expr();               // índice
            match(sFECHACOL);
            int base_offset = symtab_get_offset(id);
            if (base_offset == -1) {
                diag_error(current.line, "vetor declarado", id);
            } else {
                emit_crct(base_offset);
                emit_soma();            // endereço = base + índice
                emit_crvi();            // carrega valor do endereço
            }
            type = TIPO_INT;
        } else {
            /* Variável simples */
            int offset = symtab_get_offset(id);
            if (offset == -1) {
                diag_error(current.line, "variável declarada", id);
            } else {
                emit_crvl(offset);
            }
            Symbol *s = symtab_lookup(id);
            type = s ? s->type : TIPO_INT;
        }
        free(id);
    } else if (current.type == sABREPAR) {
        advance();
        type = parse_expr();
        match(sFECHAPAR);
    } else if (current.type == sSUBTRAT) {
        /* Menos unário */
        advance();
        parse_factor();
        emit_crct(0);
        emit_subt();
        type = TIPO_INT;
    } else if (current.type == sNEG) {
        /* Negação lógica unária */
        advance();
        parse_factor();
        emit_invr();
        type = TIPO_BOOL;
    } else {
        diag_error(current.line, "fator", token_type_name(current.type));
        advance();
        type = TIPO_INT;
    }
    diag_info("Saindo de parse_factor");
    return type;
}

/* Termo: fator (* ou / fator) */
static DataType parse_term(void) {
    diag_info("Entrando em parse_term");
    DataType t = parse_factor();
    while (current.type == sMULT || current.type == sDIV) {
        TokenType op = current.type;
        advance();
        DataType t2 = parse_factor();
        if (t != TIPO_INT || t2 != TIPO_INT)
            diag_error(current.line, "operandos inteiros", "tipos incompatíveis");
        if (op == sMULT) emit_mult();
        else emit_divi();
        t = TIPO_INT;
    }
    diag_info("Saindo de parse_term");
    return t;
}

/* Expressão simples: termo (+ ou - termo) */
static DataType parse_simple_expr(void) {
    diag_info("Entrando em parse_simple_expr");
    DataType t = parse_term();
    while (current.type == sSOMA || current.type == sSUBTRAT) {
        TokenType op = current.type;
        advance();
        DataType t2 = parse_term();
        if (t != TIPO_INT || t2 != TIPO_INT)
            diag_error(current.line, "operandos inteiros", "tipos incompatíveis");
        if (op == sSOMA) emit_soma();
        else emit_subt();
        t = TIPO_INT;
    }
    diag_info("Saindo de parse_simple_expr");
    return t;
}

/* Expressão relacional: simples (op relacional simples) */
static DataType parse_relational(void) {
    diag_info("Entrando em parse_relational");
    DataType t = parse_simple_expr();
    if (current.type == sMENOR || current.type == sMENORIG ||
        current.type == sMAIOR || current.type == sMAIORIG ||
        current.type == sIGUAL || current.type == sDIFERENTE) {
        TokenType op = current.type;
        advance();
        DataType t2 = parse_simple_expr();
        if (t != TIPO_INT || t2 != TIPO_INT)
            diag_error(current.line, "operandos inteiros", "tipos incompatíveis");
        switch (op) {
            case sMENOR:    emit_cmme(); break;
            case sMENORIG:  emit_cmmeq(); break;
            case sMAIOR:    emit_cmma(); break;
            case sMAIORIG:  emit_cmmaq(); break;
            case sIGUAL:    emit_cmeq(); break;
            case sDIFERENTE: emit_cdifeq(); break;
            default: break;
        }
        t = TIPO_BOOL;
    }
    diag_info("Saindo de parse_relational");
    return t;
}

/* Expressão lógica AND (conjunção) */
static DataType parse_logical_and(void) {
    diag_info("Entrando em parse_logical_and");
    DataType t = parse_relational();
    while (current.type == sCONJ) {
        advance();
        DataType t2 = parse_relational();
        if (t != TIPO_BOOL || t2 != TIPO_BOOL)
            diag_error(current.line, "operandos lógicos", "tipos incompatíveis");
        emit_conj();
        t = TIPO_BOOL;
    }
    diag_info("Saindo de parse_logical_and");
    return t;
}

/* Expressão lógica OR (disjunção) – nível mais alto */
static DataType parse_expr(void) {
    diag_info("Entrando em parse_expr");
    DataType t = parse_logical_and();
    while (current.type == sDISJ) {
        advance();
        DataType t2 = parse_logical_and();
        if (t != TIPO_BOOL || t2 != TIPO_BOOL)
            diag_error(current.line, "operandos lógicos", "tipos incompatíveis");
        emit_disj();
        t = TIPO_BOOL;
    }
    diag_info("Saindo de parse_expr");
    return t;
}

/* Ponto de entrada principal do parser */
void parse_program(FILE *source, const char *source_filename, int trace_enabled, int tk_flag, int st_flag) {
    diag_info("Entrando em parse_program");
    trace_flag = trace_enabled;
    tokens_flag = tk_flag;
    src_base = source_filename;

    lex_init(source);
    symtab_init();
    codegen_init(source_filename);
    symtab_enter_scope("global");
    global_offset = 0;
    current = lex_next();

    match(sMODULE);
    match(sIDENTIF);
    match(sPONTOVIRG);

    if (current.type == sGLOBALS) {
        parse_globals();
    }

    /* Desvia para o ponto de entrada, pulando as sub-rotinas */
    char *start_label = new_label();
    emit_dsvs(start_label);

    if (current.type == sPROC || current.type == sFN) {
        parse_subs();
    }

    /* Código principal: chama main */
    emit_label(start_label);
    const char *main_label = symtab_get_label("main");
    if (main_label) {
        emit_para(0);
        emit_cham(main_label);
    } else {
        diag_error(0, "procedimento main", "não encontrado");
    }
    emit_retorno();
    free(start_label);

    if (current.type != sEOF) {
        diag_error(current.line, "fim de arquivo", token_type_name(current.type));
    }

    /* Gera arquivo .ts se solicitado */
    if (st_flag) {
        char fname[512];
        snprintf(fname, sizeof(fname), "%s.ts", source_filename);
        FILE *f = fopen(fname, "w");
        if (f) {
            symtab_print(f);
            fclose(f);
        }
    }

    symtab_cleanup();
    lex_cleanup();
    codegen_finalize();
    diag_info("Saindo de parse_program");
}
