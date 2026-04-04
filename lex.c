/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _CRT_SECURE_NO_WARNINGS
#define _POSIX_C_SOURCE 200809L
#ifdef _WIN32
#define strdup _strdup
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"
#include "diag.h"

static FILE *source_file = NULL;
static int current_char = 0;
static int current_line = 1;
static int eof_reached = 0;

// Avança para o próximo caractere do arquivo
static void next_char(void) {
    if (eof_reached) return;
    int c = fgetc(source_file);
    if (c == EOF) {
        eof_reached = 1;
        current_char = '\0';
    } else {
        current_char = c;
        if (current_char == '\n') current_line++;
    }
}

// Pula comentário de bloco @{ ... }@
static void skip_block_comment(void) {
    next_char(); // consome o '{' após '@'
    while (!eof_reached) {
        if (current_char == '}') {
            next_char();
            if (current_char == '@') {
                next_char();
                return;
            }
        } else {
            next_char();
        }
    }
    if (eof_reached)
        diag_error(current_line, "fechamento de comentário }@", "EOF");
}

// Pula comentário de linha @ até o fim da linha
static void skip_line_comment(void) {
    while (current_char != '\n' && !eof_reached)
        next_char();
    if (current_char == '\n') next_char();
}

// Reconhece identificador ou palavra reservada
static Token read_identifier(void) {
    char buffer[256];
    int pos = 0;
    int line = current_line;

    while (isalnum(current_char) || current_char == '_') {
        if (pos < 255) buffer[pos++] = (char)current_char;
        next_char();
    }
    buffer[pos] = '\0';

    TokenType type = sIDENTIF;
    // Palavras reservadas da SAL (conforme especificação)
    if (strcmp(buffer, "module") == 0) type = sMODULE;
    else if (strcmp(buffer, "proc") == 0) type = sPROC;
    else if (strcmp(buffer, "fn") == 0) type = sFN;
    else if (strcmp(buffer, "globals") == 0) type = sGLOBALS;
    else if (strcmp(buffer, "locals") == 0) type = sLOCALS;
    else if (strcmp(buffer, "start") == 0) type = sSTART;
    else if (strcmp(buffer, "end") == 0) type = sEND;
    else if (strcmp(buffer, "if") == 0) type = sIF;
    else if (strcmp(buffer, "else") == 0) type = sELSE;
    else if (strcmp(buffer, "match") == 0) type = sMATCH;
    else if (strcmp(buffer, "when") == 0) type = sWHEN;
    else if (strcmp(buffer, "otherwise") == 0) type = sOTHERWISE;
    else if (strcmp(buffer, "for") == 0) type = sFOR;
    else if (strcmp(buffer, "to") == 0) type = sTO;
    else if (strcmp(buffer, "step") == 0) type = sSTEP;
    else if (strcmp(buffer, "do") == 0) type = sDO;
    else if (strcmp(buffer, "loop") == 0) type = sLOOP;
    else if (strcmp(buffer, "while") == 0) type = sWHILE;
    else if (strcmp(buffer, "until") == 0) type = sUNTIL;
    else if (strcmp(buffer, "print") == 0) type = sPRINT;
    else if (strcmp(buffer, "scan") == 0) type = sSCAN;
    else if (strcmp(buffer, "ret") == 0) type = sRET;
    else if (strcmp(buffer, "true") == 0) type = sTRUE;
    else if (strcmp(buffer, "false") == 0) type = sFALSE;
    else if (strcmp(buffer, "int") == 0) type = sINT;
    else if (strcmp(buffer, "bool") == 0) type = sBOOL;
    else if (strcmp(buffer, "char") == 0) type = sCHAR;

    return (Token){type, strdup(buffer), line};
}

// Reconhece número inteiro
static Token read_number(void) {
    char buffer[32];
    int pos = 0;
    int line = current_line;

    while (isdigit(current_char)) {
        if (pos < 31) buffer[pos++] = (char)current_char;
        next_char();
    }
    buffer[pos] = '\0';
    return (Token){sCTEINT, strdup(buffer), line};
}

// Reconhece string entre aspas duplas
static Token read_string(void) {
    char buffer[1024];
    int pos = 0;
    int line = current_line;
    next_char(); // consome a aspa inicial
    while (current_char != '"' && !eof_reached && current_char != '\n') {
        if (pos < 1023) buffer[pos++] = (char)current_char;
        next_char();
    }
    if (current_char != '"')
        diag_error(line, "aspas fechando string", current_char == '\n' ? "quebra de linha" : "EOF");
    else
        next_char(); // consome aspa final
    buffer[pos] = '\0';
    return (Token){sSTRING, strdup(buffer), line};
}

// Reconhece caractere literal entre aspas simples
static Token read_char_literal(void) {
    int line = current_line;
    next_char(); // consome aspa simples inicial
    char ch = (char)current_char;
    next_char();
    if (current_char != '\'')
        diag_error(line, "aspas simples fechando", "caractere inesperado");
    else
        next_char();
    char buf[2] = {ch, '\0'};
    return (Token){sCTECHAR, strdup(buf), line};
}

// Inicializa o analisador léxico
void lex_init(FILE *source) {
    source_file = source;
    current_line = 1;
    eof_reached = 0;
    next_char();
}

// Retorna o próximo token do código fonte
Token lex_next(void) {
    // Ignora espaços em branco
    while (isspace(current_char) && current_char != '\n')
        next_char();
    if (current_char == '\n') {
        next_char();
        return lex_next();
    }

    if (eof_reached || current_char == '\0')
        return (Token){sEOF, strdup(""), current_line};

    // Comentários
    if (current_char == '@') {
        next_char();
        if (current_char == '{') {
            skip_block_comment();
            return lex_next();
        } else {
            // comentário de linha (apenas @, sem {)
            skip_line_comment();
            return lex_next();
        }
    }

    if (isalpha(current_char) || current_char == '_')
        return read_identifier();
    if (isdigit(current_char))
        return read_number();
    if (current_char == '"')
        return read_string();
    if (current_char == '\'')
        return read_char_literal();

    int line = current_line;
    char c = (char)current_char;
    next_char();

    switch (c) {
        case ':':
            if (current_char == '=') { next_char(); return (Token){sATRIB, strdup(":="), line}; }
            return (Token){sDOISPONTOS, strdup(":"), line};
        case '=':
            if (current_char == '>') { next_char(); return (Token){sIMPLIC, strdup("=>"), line}; }
            return (Token){sIGUAL, strdup("="), line};
        case '<':
            if (current_char == '>') { next_char(); return (Token){sDIFERENTE, strdup("<>"), line}; }
            if (current_char == '=') { next_char(); return (Token){sMENORIG, strdup("<="), line}; }
            return (Token){sMENOR, strdup("<"), line};
        case '>':
            if (current_char == '=') { next_char(); return (Token){sMAIORIG, strdup(">="), line}; }
            return (Token){sMAIOR, strdup(">"), line};
        case '+': return (Token){sSOMA, strdup("+"), line};
        case '-': return (Token){sSUBTRAT, strdup("-"), line};
        case '*': return (Token){sMULT, strdup("*"), line};
        case '/': return (Token){sDIV, strdup("/"), line};
        case '^': return (Token){sCONJ, strdup("^"), line};
        case 'v': return (Token){sDISJ, strdup("v"), line};
        case '~': return (Token){sNEG, strdup("~"), line};
        case '(': return (Token){sABREPAR, strdup("("), line};
        case ')': return (Token){sFECHAPAR, strdup(")"), line};
        case '[': return (Token){sABRECOL, strdup("["), line};
        case ']': return (Token){sFECHACOL, strdup("]"), line};
        case ',': return (Token){sVIRG, strdup(","), line};
        case ';': return (Token){sPONTOVIRG, strdup(";"), line};
        case '.':
            if (current_char == '.') { next_char(); return (Token){sPTOPTO, strdup(".."), line}; }
            return (Token){sERROR, strdup("."), line};
        default: {
            char err[2] = {c, '\0'};
            diag_error(line, "token válido", err);
            return (Token){sERROR, strdup(err), line};
        }
    }
}

// Retorna o nome da categoria do token para uso em logs e mensagens
const char* token_type_name(TokenType t) {
    switch (t) {
        case sMODULE: return "sMODULE";
        case sPROC: return "sPROC";
        case sFN: return "sFN";
        case sGLOBALS: return "sGLOBALS";
        case sLOCALS: return "sLOCALS";
        case sSTART: return "sSTART";
        case sEND: return "sEND";
        case sIF: return "sIF";
        case sELSE: return "sELSE";
        case sMATCH: return "sMATCH";
        case sWHEN: return "sWHEN";
        case sOTHERWISE: return "sOTHERWISE";
        case sFOR: return "sFOR";
        case sTO: return "sTO";
        case sSTEP: return "sSTEP";
        case sDO: return "sDO";
        case sLOOP: return "sLOOP";
        case sWHILE: return "sWHILE";
        case sUNTIL: return "sUNTIL";
        case sPRINT: return "sPRINT";
        case sSCAN: return "sSCAN";
        case sRET: return "sRET";
        case sTRUE: return "sTRUE";
        case sFALSE: return "sFALSE";
        case sINT: return "sINT";
        case sBOOL: return "sBOOL";
        case sCHAR: return "sCHAR";
        case sATRIB: return "sATRIB";
        case sIGUAL: return "sIGUAL";
        case sDIFERENTE: return "sDIFERENTE";
        case sMENOR: return "sMENOR";
        case sMENORIG: return "sMENORIG";
        case sMAIOR: return "sMAIOR";
        case sMAIORIG: return "sMAIORIG";
        case sSOMA: return "sSOMA";
        case sSUBTRAT: return "sSUBTRAT";
        case sMULT: return "sMULT";
        case sDIV: return "sDIV";
        case sCONJ: return "sCONJ";
        case sDISJ: return "sDISJ";
        case sNEG: return "sNEG";
        case sABREPAR: return "sABREPAR";
        case sFECHAPAR: return "sFECHAPAR";
        case sABRECOL: return "sABRECOL";
        case sFECHACOL: return "sFECHACOL";
        case sVIRG: return "sVIRG";
        case sPONTOVIRG: return "sPONTOVIRG";
        case sDOISPONTOS: return "sDOISPONTOS";
        case sPTOPTO: return "sPTOPTO";
        case sIMPLIC: return "sIMPLIC";
        case sIDENTIF: return "sIDENTIF";
        case sCTEINT: return "sCTEINT";
        case sCTECHAR: return "sCTECHAR";
        case sSTRING: return "sSTRING";
        case sEOF: return "sEOF";
        default: return "sERROR";
    }
}

// Limpeza (não precisa fechar o arquivo, pois é feito no main)
void lex_cleanup(void) {
    source_file = NULL;
}
