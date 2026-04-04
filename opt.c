/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _CRT_SECURE_NO_WARNINGS
#define _POSIX_C_SOURCE 200809L
#ifdef _WIN32
#define strdup _strdup
#endif
#include <string.h>
#include "opt.h"
#include <stdio.h>
#include <stdlib.h>

void opt_parse(int argc, char *argv[], Options *opts) {
    memset(opts, 0, sizeof(Options));
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.sal> [--tokens] [--symtab] [--trace]\n", argv[0]);
        exit(1);
    }
    opts->source_file = _strdup(argv[1]);
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0)
            opts->tokens_flag = 1;
        else if (strcmp(argv[i], "--symtab") == 0)
            opts->symtab_flag = 1;
        else if (strcmp(argv[i], "--trace") == 0)
            opts->trace_flag = 1;
        else {
            fprintf(stderr, "Opção desconhecida: %s\n", argv[i]);
            exit(1);
        }
    }
}

void opt_cleanup(Options *opts) {
    free(opts->source_file);
}
