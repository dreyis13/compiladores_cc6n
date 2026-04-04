/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#define _CRT_SECURE_NO_WARNINGS
#define _POSIX_C_SOURCE 200809L
#ifdef _WIN32
#define strdup _strdup
#endif
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static FILE *output = NULL;
static int label_counter = 0;

void codegen_init(const char *filename) {
    char fname[512];
    snprintf(fname, sizeof(fname), "%s.mepa", filename);
    output = fopen(fname, "w");
    if (!output) {
        fprintf(stderr, "Erro ao criar arquivo %s\n", fname);
        exit(1);
    }
}

void codegen_emit(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(output, fmt, args);
    fprintf(output, "\n");
    va_end(args);
}

void codegen_finalize(void) {
    if (output) fclose(output);
}

char* new_label(void) {
    char *label = malloc(32);
    sprintf(label, "L%d", label_counter++);
    return label;
}

void emit_amem(int n) { codegen_emit("AMEM %d", n); }
void emit_dmem(int n) { codegen_emit("DMEM %d", n); }
void emit_crct(int value) { codegen_emit("CRCT %d", value); }
void emit_crvl(int offset) { codegen_emit("CRVL %d", offset); }
void emit_armz(int offset) { codegen_emit("ARMZ %d", offset); }
void emit_soma(void) { codegen_emit("SOMA"); }
void emit_subt(void) { codegen_emit("SUBT"); }
void emit_mult(void) { codegen_emit("MULT"); }
void emit_divi(void) { codegen_emit("DIVI"); }
void emit_invr(void) { codegen_emit("INVR"); }
void emit_conj(void) { codegen_emit("CONJ"); }
void emit_disj(void) { codegen_emit("DISJ"); }
void emit_cmme(void) { codegen_emit("CMME"); }
void emit_cmma(void) { codegen_emit("CMMA"); }
void emit_cmeq(void) { codegen_emit("CMEQ"); }
void emit_cdifeq(void) { codegen_emit("CDIF"); }
void emit_cmmeq(void) { codegen_emit("CMMEQ"); }
void emit_cmmaq(void) { codegen_emit("CMMAQ"); }
void emit_dsvs(const char *label) { codegen_emit("DSVS %s", label); }
void emit_dsvf(const char *label) { codegen_emit("DSVF %s", label); }
void emit_chpr(void) { codegen_emit("CHPR"); }
void emit_chle(void) { codegen_emit("CHLE"); }
void emit_para(int n) { codegen_emit("PARA %d", n); }
void emit_cham(const char *label) { codegen_emit("CHAM %s", label); }
void emit_retorno(void) { codegen_emit("RETOR"); }
void emit_label(const char *label) { codegen_emit("%s:", label); }

// Novas instruções para vetores
void emit_crvi(void) { codegen_emit("CRVI"); }
void emit_armi(void) { codegen_emit("ARMI"); }
void emit_troca(void) { codegen_emit("TROCA"); }
void emit_chpd(void) { codegen_emit("CHPD"); }
