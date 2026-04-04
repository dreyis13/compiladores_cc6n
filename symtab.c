/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _CRT_SECURE_NO_WARNINGS
#define _POSIX_C_SOURCE 200809L
#ifdef _WIN32
#define strdup _strdup
#endif
#include "symtab.h"
#include "diag.h"
#include <stdlib.h>
#include <string.h>

static Scope *current_scope = NULL;
static int block_counter = 0;
static int total_locals = 0;

static Scope* new_scope(const char *desc) {
    Scope *s = (Scope*)malloc(sizeof(Scope));
    s->description = strdup(desc);
    s->symbols = NULL;
    s->prev = current_scope;
    return s;
}

void symtab_init(void) {
    current_scope = NULL;
    block_counter = 0;
    total_locals = 0;
}

void symtab_enter_scope(const char *description) {
    current_scope = new_scope(description);
    diag_info("Entrou no escopo: %s", description);
}

void symtab_exit_scope(void) {
    if (!current_scope) return;
    diag_info("Saiu do escopo: %s", current_scope->description);
    Symbol *sym = current_scope->symbols;
    while (sym) {
        Symbol *next = sym->next;
        free(sym->name);
        if (sym->label) free(sym->label);
        free(sym);
        sym = next;
    }
    free(current_scope->description);
    Scope *prev = current_scope->prev;
    free(current_scope);
    current_scope = prev;
}

int symtab_insert(const char *name, SymCategory cat, DataType type, int extra) {
    if (!current_scope) return 0;
    for (Symbol *s = current_scope->symbols; s; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            diag_error(0, "símbolo não duplicado no mesmo escopo", name);
            return 0;
        }
    }
    Symbol *new_sym = (Symbol*)malloc(sizeof(Symbol));
    new_sym->name = strdup(name);
    new_sym->category = cat;
    new_sym->type = type;
    new_sym->extra = extra;
    new_sym->offset = -1;
    new_sym->label = NULL;
    new_sym->next = current_scope->symbols;
    current_scope->symbols = new_sym;
    return 1;
}

Symbol* symtab_lookup(const char *name) {
    Scope *sc = current_scope;
    while (sc) {
        for (Symbol *s = sc->symbols; s; s = s->next)
            if (strcmp(s->name, name) == 0) return s;
        sc = sc->prev;
    }
    return NULL;
}

Scope* symtab_get_current_scope(void) {
    return current_scope;
}

static const char* cat_to_string(SymCategory cat) {
    switch(cat) {
        case CAT_VAR: return "var";
        case CAT_VECTOR: return "vector";
        case PARAM: return "param";
        case PROC: return "proc";
        case FUNC: return "func";
        default: return "unknown";
    }
}

static const char* type_to_string(DataType type) {
    switch(type) {
        case TIPO_INT: return "int";
        case TIPO_BOOL: return "bool";
        case TIPO_CHAR: return "char";
        default: return "unknown";
    }
}

void symtab_print(FILE *out) {
    if (!out) return;
    int count = 0;
    for (Scope *sc = current_scope; sc; sc = sc->prev) count++;
    Scope **scopes = (Scope**)malloc(count * sizeof(Scope*));
    Scope *sc = current_scope;
    for (int i = count-1; i >= 0; i--) {
        scopes[i] = sc;
        sc = sc->prev;
    }
    for (int i = 0; i < count; i++) {
        Scope *s = scopes[i];
        fprintf(out, "SCOPE=%s\n", s->description);
        // inverter ordem para inserção
        Symbol *rev = NULL, *cur = s->symbols;
        while (cur) {
            Symbol *next = cur->next;
            cur->next = rev;
            rev = cur;
            cur = next;
        }
        for (cur = rev; cur; cur = cur->next) {
            fprintf(out, "  id=\"%s\" cat=%s tipo=%s extra=%d\n",
                    cur->name, cat_to_string(cur->category),
                    type_to_string(cur->type), cur->extra);
        }
    }
    free(scopes);
}

void symtab_update_extra(const char *name, int extra) {
    Symbol *s = symtab_lookup(name);
    if (s) s->extra = extra;
}

void symtab_cleanup(void) {
    while (current_scope)
        symtab_exit_scope();
}

void symtab_set_offset(const char *name, int offset) {
    Symbol *s = symtab_lookup(name);
    if (s) s->offset = offset;
}

int symtab_get_offset(const char *name) {
    Symbol *s = symtab_lookup(name);
    return s ? s->offset : -1;
}

int symtab_get_param_count(const char *name) {
    Symbol *s = symtab_lookup(name);
    return s ? s->extra : 0;
}

void symtab_set_label(const char *name, const char *label) {
    Symbol *s = symtab_lookup(name);
    if (s) {
        if (s->label) free(s->label);
        s->label = strdup(label);
    }
}

const char* symtab_get_label(const char *name) {
    Symbol *s = symtab_lookup(name);
    return s ? s->label : NULL;
}

void symtab_set_total_locals(int size) {
    total_locals = size;
}

int symtab_get_total_locals(void) {
    return total_locals;
}
