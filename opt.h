/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef OPT_H
#define OPT_H

/* Estrutura que armazena as opções passadas pela linha de comando */
typedef struct {
    char *source_file;    // nome do arquivo fonte .sal
    int tokens_flag;      // ativa geração do arquivo .tk (lista de tokens)
    int symtab_flag;      // ativa geração do arquivo .ts (tabela de símbolos)
    int trace_flag;       // ativa geração do arquivo .trc (rastreamento)
} Options;

/* Processa os argumentos da linha de comando e preenche a estrutura */
void opt_parse(int argc, char *argv[], Options *opts);

/* Libera a memória alocada para o nome do arquivo fonte */
void opt_cleanup(Options *opts);

#endif
