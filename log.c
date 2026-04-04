/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _CRT_SECURE_NO_WARNINGS
#include "log.h"
#include "lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Arquivos de saída para os logs */
static FILE *token_file = NULL;   // arquivo .tk (lista de tokens)
static FILE *trace_file = NULL;   // arquivo .trc (rastreamento da análise)

/* Abre o arquivo para gravar a lista de tokens (.tk) */
void log_open_tokens(const char *base_filename) {
    char fname[512];
    snprintf(fname, sizeof(fname), "%s.tk", base_filename);
    token_file = fopen(fname, "w");
    if (!token_file) {
        fprintf(stderr, "Erro ao criar %s\n", fname);
    }
}

/* Escreve um token no arquivo .tk no formato exigido: linha, categoria e lexema */
void log_write_token(Token tok) {
    if (!token_file) return;
    fprintf(token_file, "%d  %s  \"%s\"\n", tok.line, token_type_name(tok.type), tok.lexeme);
    fflush(token_file); // garante gravação imediata
}

/* Fecha o arquivo de tokens */
void log_close_tokens(void) {
    if (token_file) fclose(token_file);
    token_file = NULL;
}

/* Abre o arquivo para gravar o rastreamento (.trc) */
void log_open_trace(const char *base_filename) {
    char fname[512];
    snprintf(fname, sizeof(fname), "%s.trc", base_filename);
    trace_file = fopen(fname, "w");
    if (!trace_file) {
        fprintf(stderr, "Erro ao criar %s\n", fname);
    }
}

/* Escreve uma mensagem de rastreamento no arquivo .trc */
void log_write_trace(const char *msg) {
    if (!trace_file) return;
    fprintf(trace_file, "%s\n", msg);
    fflush(trace_file);
}

/* Fecha o arquivo de rastreamento */
void log_close_trace(void) {
    if (trace_file) fclose(trace_file);
    trace_file = NULL;
}
