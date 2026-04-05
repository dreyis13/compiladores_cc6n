/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _CRT_SECURE_NO_WARNINGS
#define _POSIX_C_SOURCE 200809L
#ifdef _WIN32
#define strdup _strdup
#endif
#include "symtab.h"
#include "diag.h"
#include <stdlib.h>
#include <string.h>

/* Escopo atual da tabela de símbolos (pilha de escopos) */
static Scope *current_scope = NULL;
static int block_counter = 0;      // contador de blocos (não usado atualmente)
static int total_locals = 0;       // total de variáveis locais da sub-rotina atual

/* Cria um novo escopo e o empilha */
static Scope* new_scope(const char *desc) {
    Scope *s = (Scope*)malloc(sizeof(Scope));
    s->description = strdup(desc);
    s->symbols = NULL;
    s->prev = current_scope;
    return s;
}

/* Inicializa o módulo da tabela de símbolos */
void symtab_init(void) {
    current_scope = NULL;
    block_counter = 0;
    total_locals = 0;
}

/* Entra em um novo escopo (global, locals de sub-rotina, etc.) */
void symtab_enter_scope(const char *description) {
    current_scope = new_scope(description);
    diag_info("Entrou no escopo: %s", description);
}

/* Sai do escopo atual, liberando todos os símbolos contidos nele */
void symtab_exit_scope(void) {
    if (!current_scope) return;
    diag_info("Saiu do escopo: %s", current_scope->description);
    /* Libera todos os símbolos do escopo */
    Symbol *sym = current_scope->symbols;
    while (sym) {
        Symbol *next = sym->next;
        free(sym->name);
        if (sym->label) free(sym->label);
        free(sym);
        sym = next;
    }
    free(current_scope->description);
    Scope *prev = current_scope->prev;
    free(current_scope);
    current_scope = prev;
}

/* Insere um símbolo no escopo atual. Retorna 1 se sucesso, 0 se duplicado */
int symtab_insert(const char *name, SymCategory cat, DataType type, int extra) {
    if (!current_scope) return 0;
    /* Verifica duplicata no escopo atual */
    for (Symbol *s = current_scope->symbols; s; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            diag_error(0, "símbolo não duplicado no mesmo escopo", name);
            return 0;
        }
    }
    Symbol *new_sym = (Symbol*)malloc(sizeof(Symbol));
    new_sym->name = strdup(name);
    new_sym->category = cat;
    new_sym->type = type;
    new_sym->extra = extra;
    new_sym->offset = -1;        // ainda não definido
    new_sym->label = NULL;       // ainda não definido
    new_sym->next = current_scope->symbols;
    current_scope->symbols = new_sym;
    return 1;
}

/* Busca um símbolo pelo nome, respeitando a cadeia de escopos (do mais interno para o externo) */
Symbol* symtab_lookup(const char *name) {
    Scope *sc = current_scope;
    while (sc) {
        for (Symbol *s = sc->symbols; s; s = s->next)
            if (strcmp(s->name, name) == 0) return s;
        sc = sc->prev;
    }
    return NULL;
}

/* Retorna o escopo atual (para acesso externo) */
Scope* symtab_get_current_scope(void) {
    return current_scope;
}

/* Converte categoria do símbolo em string para impressão */
static const char* cat_to_string(SymCategory cat) {
    switch(cat) {
        case CAT_VAR: return "var";
        case CAT_VECTOR: return "vector";
        case PARAM: return "param";
        case PROC: return "proc";
        case FUNC: return "func";
        default: return "unknown";
    }
}

/* Converte tipo de dado em string para impressão */
static const char* type_to_string(DataType type) {
    switch(type) {
        case TIPO_INT: return "int";
        case TIPO_BOOL: return "bool";
        case TIPO_CHAR: return "char";
        default: return "unknown";
    }
}

/* Imprime a tabela de símbolos no arquivo de saída, no formato exigido */
void symtab_print(FILE *out) {
    if (!out) return;
    /* Coleta todos os escopos na ordem (do mais externo para o mais interno) */
    int count = 0;
    for (Scope *sc = current_scope; sc; sc = sc->prev) count++;
    Scope **scopes = (Scope**)malloc(count * sizeof(Scope*));
    Scope *sc = current_scope;
    for (int i = count-1; i >= 0; i--) {
        scopes[i] = sc;
        sc = sc->prev;
    }
    /* Para cada escopo, imprime seus símbolos na ordem de inserção */
    for (int i = 0; i < count; i++) {
        Scope *s = scopes[i];
        fprintf(out, "SCOPE=%s\n", s->description);
        /* Inverte a lista ligada para preservar a ordem de inserção */
        Symbol *rev = NULL, *cur = s->symbols;
        while (cur) {
            Symbol *next = cur->next;
            cur->next = rev;
            rev = cur;
            cur = next;
        }
        for (cur = rev; cur; cur = cur->next) {
            fprintf(out, "  id=\"%s\" cat=%s tipo=%s extra=%d\n",
                    cur->name, cat_to_string(cur->category),
                    type_to_string(cur->type), cur->extra);
        }
    }
    free(scopes);
}

/* Atualiza o campo 'extra' de um símbolo existente */
void symtab_update_extra(const char *name, int extra) {
    Symbol *s = symtab_lookup(name);
    if (s) s->extra = extra;
}

/* Remove todos os escopos e libera memória */
void symtab_cleanup(void) {
    while (current_scope)
        symtab_exit_scope();
}

/* Define o offset (posição na pilha) de um símbolo */
void symtab_set_offset(const char *name, int offset) {
    Symbol *s = symtab_lookup(name);
    if (s) s->offset = offset;
}

/* Retorna o offset de um símbolo (ou -1 se não encontrado) */
int symtab_get_offset(const char *name) {
    Symbol *s = symtab_lookup(name);
    return s ? s->offset : -1;
}

/* Retorna o número de parâmetros (extra) de uma sub-rotina */
int symtab_get_param_count(const char *name) {
    Symbol *s = symtab_lookup(name);
    return s ? s->extra : 0;
}

/* Define o rótulo associado a um símbolo (para procedimentos/funções) */
void symtab_set_label(const char *name, const char *label) {
    Symbol *s = symtab_lookup(name);
    if (s) {
        if (s->label) free(s->label);
        s->label = strdup(label);
    }
}

/* Retorna o rótulo de um símbolo (ou NULL) */
const char* symtab_get_label(const char *name) {
    Symbol *s = symtab_lookup(name);
    return s ? s->label : NULL;
}

/* Armazena o total de variáveis locais da sub-rotina atual */
void symtab_set_total_locals(int size) {
    total_locals = size;
}

/* Retorna o total de variáveis locais da sub-rotina atual */
int symtab_get_total_locals(void) {
    return total_locals;
}
