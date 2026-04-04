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

static FILE *output = NULL;       // Arquivo de saída .mepa
static int label_counter = 0;     // Contador para gerar rótulos únicos

// Inicializa a geração de código, criando o arquivo .mepa
void codegen_init(const char *filename) {
    char fname[512];
    snprintf(fname, sizeof(fname), "%s.mepa", filename);
    output = fopen(fname, "w");
    if (!output) {
        fprintf(stderr, "Erro ao criar arquivo %s\n", fname);
        exit(1);
    }
}

// Escreve uma instrução formatada no arquivo .mepa
void codegen_emit(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(output, fmt, args);
    fprintf(output, "\n");
    va_end(args);
}

// Fecha o arquivo .mepa
void codegen_finalize(void) {
    if (output) fclose(output);
}

// Gera um rótulo único para desvios (ex: L0, L1, ...)
char* new_label(void) {
    char *label = malloc(32);
    sprintf(label, "L%d", label_counter++);
    return label;
}

// Instruções de gerenciamento de memória na pilha
void emit_amem(int n) { codegen_emit("AMEM %d", n); }   // aloca n células
void emit_dmem(int n) { codegen_emit("DMEM %d", n); }   // desaloca n células

// Constantes e acesso a variáveis
void emit_crct(int value) { codegen_emit("CRCT %d", value); }     // empilha constante
void emit_crvl(int offset) { codegen_emit("CRVL %d", offset); }   // carrega variável local/global
void emit_armz(int offset) { codegen_emit("ARMZ %d", offset); }   // armazena topo na variável

// Operações aritméticas
void emit_soma(void) { codegen_emit("SOMA"); }
void emit_subt(void) { codegen_emit("SUBT"); }
void emit_mult(void) { codegen_emit("MULT"); }
void emit_divi(void) { codegen_emit("DIVI"); }

// Operações lógicas e negação
void emit_invr(void) { codegen_emit("INVR"); }   // negação (~)
void emit_conj(void) { codegen_emit("CONJ"); }   // conjunção (^)
void emit_disj(void) { codegen_emit("DISJ"); }   // disjunção (v)

// Comparações relacionais
void emit_cmme(void) { codegen_emit("CMME"); }     // <
void emit_cmma(void) { codegen_emit("CMMA"); }     // >
void emit_cmeq(void) { codegen_emit("CMEQ"); }     // =
void emit_cdifeq(void) { codegen_emit("CDIF"); }   // <>
void emit_cmmeq(void) { codegen_emit("CMMEQ"); }   // <=
void emit_cmmaq(void) { codegen_emit("CMMAQ"); }   // >=

// Desvios condicionais e incondicionais
void emit_dsvs(const char *label) { codegen_emit("DSVS %s", label); }   // desvio incondicional
void emit_dsvf(const char *label) { codegen_emit("DSVF %s", label); }   // desvio se falso

// Entrada e saída
void emit_chpr(void) { codegen_emit("CHPR"); }   // imprime caractere (topo da pilha)
void emit_chle(void) { codegen_emit("CHLE"); }   // lê inteiro e empilha
void emit_chpd(void) { codegen_emit("CHPD"); }   // imprime número decimal

// Chamada de sub-rotinas
void emit_para(int n) { codegen_emit("PARA %d", n); }         // passa n parâmetros
void emit_cham(const char *label) { codegen_emit("CHAM %s", label); } // chama sub-rotina
void emit_retorno(void) { codegen_emit("RETOR"); }            // retorna da sub-rotina

// Rótulo (ponto de desvio)
void emit_label(const char *label) { codegen_emit("%s:", label); }

// Instruções auxiliares para vetores
void emit_crvi(void) { codegen_emit("CRVI"); }   // carrega valor do endereço no topo
void emit_armi(void) { codegen_emit("ARMI"); }   // armazena valor no endereço no topo
void emit_troca(void) { codegen_emit("TROCA"); } // troca os dois valores do topo da pilha
