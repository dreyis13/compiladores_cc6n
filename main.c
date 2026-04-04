/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _POSIX_C_SOURCE 200809L
#include "opt.h"
#include "diag.h"
#include "lex.h"
#include "parser.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    Options opts;
    opt_parse(argc, argv, &opts);
    diag_init(opts.trace_flag);

    FILE *source = fopen(opts.source_file, "r");
    if (!source) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", opts.source_file);
        return 1;
    }

    // Base name para arquivos de log (sem extensão)
    char base[512];
    strcpy(base, opts.source_file);
    char *dot = strrchr(base, '.');
    if (dot) *dot = '\0';

    if (opts.tokens_flag) log_open_tokens(base);
    if (opts.trace_flag)  log_open_trace(base);

    // Chama o parser com todos os argumentos necessários
    parse_program(source, base, opts.trace_flag, opts.tokens_flag, opts.symtab_flag);

    if (opts.tokens_flag) log_close_tokens();
    if (opts.trace_flag)  log_close_trace();

    fclose(source);
    diag_cleanup();
    opt_cleanup(&opts);

    return diag_error_count() == 0 ? 0 : 1;
}
