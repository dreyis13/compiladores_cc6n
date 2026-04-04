/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef CODEGEN_H
#define CODEGEN_H

/* Inicialização e finalização da geração de código */
void codegen_init(const char *filename);
void codegen_finalize(void);

/* Gerenciamento de memória na pilha */
void emit_amem(int n);
void emit_dmem(int n);

/* Constantes e acesso a variáveis */
void emit_crct(int value);
void emit_crvl(int offset);
void emit_armz(int offset);

/* Operações aritméticas */
void emit_soma(void);
void emit_subt(void);
void emit_mult(void);
void emit_divi(void);

/* Operações lógicas e negação */
void emit_invr(void);
void emit_conj(void);
void emit_disj(void);

/* Comparações relacionais */
void emit_cmme(void);    // <
void emit_cmma(void);    // >
void emit_cmeq(void);    // =
void emit_cdifeq(void);  // <>
void emit_cmmeq(void);   // <=
void emit_cmmaq(void);   // >=

/* Desvios condicionais e incondicionais */
void emit_dsvs(const char *label);  // salto incondicional
void emit_dsvf(const char *label);  // salto se falso

/* Entrada e saída */
void emit_chpr(void);  // imprime caractere
void emit_chle(void);  // lê inteiro
void emit_chpd(void);  // imprime número decimal

/* Chamada de sub-rotinas */
void emit_para(int n);
void emit_cham(const char *label);
void emit_retorno(void);

/* Rótulos */
void emit_label(const char *label);
char* new_label(void);

/* Instruções auxiliares para vetores */
void emit_crvi(void);   // carrega valor do endereço no topo
void emit_armi(void);   // armazena valor no endereço no topo
void emit_troca(void);  // troca os dois valores do topo da pilha

#endif
