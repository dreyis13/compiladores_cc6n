/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _CRT_SECURE_NO_WARNINGS
#include "diag.h"
#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Contador de erros e flag para ativar rastreamento */
static int error_count = 0;
static int trace_on = 0;

/* Inicializa o módulo de diagnóstico, ativando ou não o trace */
void diag_init(int trace_enabled) {
    trace_on = trace_enabled;
    error_count = 0;
}

/* Exibe mensagem informativa no stderr e também grava no arquivo .trc se o trace estiver ativo */
void diag_info(const char *format, ...) {
    if (!trace_on) return;

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    /* Grava a mesma mensagem no arquivo de trace */
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    log_write_trace(buffer);
    va_end(args);
}

/* Registra um erro sintático ou léxico: linha, token esperado e token encontrado */
void diag_error(int line, const char *expected, const char *found) {
    error_count++;
    fprintf(stderr, "Erro na linha %d: esperado '%s', encontrado '%s'\n",
            line, expected, found);
}

/* Retorna o número de erros ocorridos até o momento */
int diag_error_count(void) {
    return error_count;
}

/* Libera recursos (nada a fazer neste módulo) */
void diag_cleanup(void) {}
