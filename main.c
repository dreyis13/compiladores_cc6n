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
    /* Processa os argumentos da linha de comando (--tokens, --symtab, --trace) */
    Options opts;
    opt_parse(argc, argv, &opts);
    diag_init(opts.trace_flag);   /* ativa o sistema de diagnóstico com ou sem trace */

    /* Abre o arquivo fonte .sal */
    FILE *source = fopen(opts.source_file, "r");
    if (!source) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", opts.source_file);
        return 1;
    }

    /* Cria o nome base (sem extensão) para os arquivos de log */
    char base[512];
    strcpy(base, opts.source_file);
    char *dot = strrchr(base, '.');
    if (dot) *dot = '\0';

    /* Abre os arquivos de log conforme as flags solicitadas */
    if (opts.tokens_flag) log_open_tokens(base);
    if (opts.trace_flag)  log_open_trace(base);

    /* Executa a análise sintática completa (inclui léxico, sintaxe e tabela de símbolos) */
    parse_program(source, base, opts.trace_flag, opts.tokens_flag, opts.symtab_flag);

    /* Fecha os arquivos de log se foram abertos */
    if (opts.tokens_flag) log_close_tokens();
    if (opts.trace_flag)  log_close_trace();

    /* Finaliza os módulos e libera recursos */
    fclose(source);
    diag_cleanup();
    opt_cleanup(&opts);

    /* Retorna 0 se nenhum erro foi reportado, caso contrário 1 */
    return diag_error_count() == 0 ? 0 : 1;
}
