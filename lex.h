/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef LEX_H
#define LEX_H

#include <stdio.h>

/* Lista de todos os tipos de token da linguagem SAL */
typedef enum {
    /* Palavras reservadas */
    sMODULE, sPROC, sFN, sGLOBALS, sLOCALS, sSTART, sEND,
    sIF, sELSE, sMATCH, sWHEN, sOTHERWISE, sFOR, sTO, sSTEP, sDO,
    sLOOP, sWHILE, sUNTIL, sPRINT, sSCAN, sRET,
    sTRUE, sFALSE,
    sINT, sBOOL, sCHAR,

    /* Operadores e delimitadores */
    sATRIB,        // :=
    sIGUAL,        // =
    sDIFERENTE,    // <>
    sMENOR,        // <
    sMENORIG,      // <=
    sMAIOR,        // >
    sMAIORIG,      // >=
    sSOMA,         // +
    sSUBTRAT,      // -
    sMULT,         // *
    sDIV,          // /
    sCONJ,         // ^ (e lógico)
    sDISJ,         // v (ou lógico)
    sNEG,          // ~ (negação)
    sABREPAR,      // (
    sFECHAPAR,     // )
    sABRECOL,      // [
    sFECHACOL,     // ]
    sVIRG,         // ,
    sPONTOVIRG,    // ;
    sDOISPONTOS,   // :
    sPTOPTO,       // ..
    sIMPLIC,       // =>

    /* Literais e identificadores */
    sIDENTIF,      // identificador (variável, nome de função, etc.)
    sCTEINT,       // constante inteira
    sCTECHAR,      // constante caractere ('a')
    sSTRING,       // literal string ("...")
    sVAR,          // palavra-chave 'var' (não usada na versão atual)

    /* Especiais */
    sEOF,          // fim do arquivo
    sERROR         // token inválido/erro
} TokenType;

/* Estrutura que representa um token */
typedef struct {
    TokenType type;    // categoria do token
    char *lexeme;      // texto original do token
    int line;          // linha onde ocorre no arquivo fonte
} Token;

/* Funções públicas do analisador léxico */
void lex_init(FILE *source);               // inicializa com arquivo fonte
Token lex_next(void);                      // retorna o próximo token
void lex_cleanup(void);                    // libera recursos
const char* token_type_name(TokenType t);  // converte tipo em string (para logs)

#endif
