/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef OPT_H
#define OPT_H

typedef struct {
    char *source_file;
    int tokens_flag;
    int symtab_flag;
    int trace_flag;
} Options;

void opt_parse(int argc, char *argv[], Options *opts);
void opt_cleanup(Options *opts);

#endif
