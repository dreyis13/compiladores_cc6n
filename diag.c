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

static int error_count = 0;
static int trace_on = 0;

void diag_init(int trace_enabled) {
    trace_on = trace_enabled;
    error_count = 0;
}

void diag_info(const char *format, ...) {
    if (!trace_on) return;

    va_list args;
    va_start(args, format);
    // imprime no stderr
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    // grava no arquivo .trc via log_write_trace
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    log_write_trace(buffer);
    va_end(args);
}

void diag_error(int line, const char *expected, const char *found) {
    error_count++;
    fprintf(stderr, "Erro na linha %d: esperado '%s', encontrado '%s'\n",
            line, expected, found);
}

int diag_error_count(void) {
    return error_count;
}

void diag_cleanup(void) {}
