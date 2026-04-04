/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef LEX_H
#define LEX_H

#include <stdio.h>

typedef enum {
    // Palavras reservadas
    sMODULE, sPROC, sFN, sGLOBALS, sLOCALS, sSTART, sEND,
    sIF, sELSE, sMATCH, sWHEN, sOTHERWISE, sFOR, sTO, sSTEP, sDO,
    sLOOP, sWHILE, sUNTIL, sPRINT, sSCAN, sRET,
    sTRUE, sFALSE,
    sINT, sBOOL, sCHAR,
    // Operadores e delimitadores
    sATRIB, sIGUAL, sDIFERENTE, sMENOR, sMENORIG, sMAIOR, sMAIORIG,
    sSOMA, sSUBTRAT, sMULT, sDIV, sCONJ, sDISJ, sNEG,
    sABREPAR, sFECHAPAR, sABRECOL, sFECHACOL, sVIRG, sPONTOVIRG,
    sDOISPONTOS, sPTOPTO, sIMPLIC,
    // Literais e identificadores
    sIDENTIF, sCTEINT, sCTECHAR, sSTRING, sVAR,
    // Especiais
    sEOF, sERROR
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
} Token;

void lex_init(FILE *source);
Token lex_next(void);
void lex_cleanup(void);
const char* token_type_name(TokenType t);
#endif
