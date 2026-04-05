/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>

/* Categorias possíveis para um símbolo */
typedef enum {
    CAT_VAR,      // variável simples
    CAT_VECTOR,   // vetor (array)
    PARAM,        // parâmetro de sub-rotina
    PROC,         // procedimento
    FUNC          // função
} SymCategory;

/* Tipos de dados suportados pela linguagem */
typedef enum {
    TIPO_INT,
    TIPO_BOOL,
    TIPO_CHAR
} DataType;

/* Estrutura que representa um símbolo na tabela */
typedef struct Symbol {
    char *name;               // nome do identificador
    SymCategory category;     // categoria (var, vector, param, proc, func)
    DataType type;            // tipo (int, bool, char)
    int extra;                // para vetor: tamanho; para proc/func: número de parâmetros
    int offset;               // deslocamento na pilha (para variáveis/parâmetros)
    char *label;              // rótulo associado (para procedimentos/funções)
    struct Symbol *next;      // próximo símbolo no mesmo escopo
} Symbol;

/* Estrutura que representa um escopo (global, locals, etc.) */
typedef struct Scope {
    char *description;        // nome descritivo do escopo (ex: "global", "proc:main.locals")
    Symbol *symbols;          // lista de símbolos deste escopo
    struct Scope *prev;       // escopo anterior (pilha de escopos)
    struct Scope *next_all;   // ponteiro para lista global (não utilizado no momento)
} Scope;

/* Funções de gerenciamento de escopos */
void symtab_init(void);
void symtab_enter_scope(const char *description);
void symtab_exit_scope(void);

/* Inserção e busca de símbolos */
int symtab_insert(const char *name, SymCategory cat, DataType type, int extra);
Symbol* symtab_lookup(const char *name);
void symtab_update_extra(const char *name, int extra);

/* Impressão da tabela (para arquivo .ts) */
void symtab_print(FILE *out);

/* Limpeza completa da tabela */
void symtab_cleanup(void);

/* Funções auxiliares para geração de código */
void symtab_set_offset(const char *name, int offset);
int symtab_get_offset(const char *name);
int symtab_get_param_count(const char *name);
void symtab_set_label(const char *name, const char *label);
const char* symtab_get_label(const char *name);
void symtab_set_total_locals(int size);
int symtab_get_total_locals(void);

/* Retorna o escopo atual (pilha) */
Scope* symtab_get_current_scope(void);

#endif
