/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>

typedef enum {
    CAT_VAR,
    CAT_VECTOR,
    PARAM,
    PROC,
    FUNC
} SymCategory;

typedef enum {
    TIPO_INT,
    TIPO_BOOL,
    TIPO_CHAR
} DataType;

typedef struct Symbol {
    char *name;
    SymCategory category;
    DataType type;
    int extra;      // p/ vetor: tamanho; p/ proc/func: número de parâmetros
    int offset;     // deslocamento na pilha (para variáveis/parâmetros)
    char *label;    // rótulo para procedimentos/funções
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    char *description;
    Symbol *symbols;
    struct Scope *prev;
    struct Scope *next_all;   // para lista global de todos os escopos
} Scope;

void symtab_init(void);
void symtab_enter_scope(const char *description);
void symtab_exit_scope(void);
int symtab_insert(const char *name, SymCategory cat, DataType type, int extra);
Symbol* symtab_lookup(const char *name);
void symtab_update_extra(const char *name, int extra);
void symtab_print(FILE *out);
void symtab_cleanup(void);

// Novas funções para geração de código
void symtab_set_offset(const char *name, int offset);
int symtab_get_offset(const char *name);
int symtab_get_param_count(const char *name);
void symtab_set_label(const char *name, const char *label);
const char* symtab_get_label(const char *name);
void symtab_set_total_locals(int size);
int symtab_get_total_locals(void);
Scope* symtab_get_current_scope(void);

#endif
