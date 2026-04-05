/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

/* Função principal da análise sintática: recebe o arquivo fonte, o nome base para logs,
   e flags para ativar trace, tokens e tabela de símbolos. */
void parse_program(FILE *source, const char *source_filename, int trace_flag, int tokens_flag, int symtab_flag);

#endif
