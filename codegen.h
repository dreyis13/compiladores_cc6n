/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef CODEGEN_H
#define CODEGEN_H

void codegen_init(const char *filename);
void codegen_finalize(void);
void emit_amem(int n);
void emit_dmem(int n);
void emit_crct(int value);
void emit_crvl(int offset);
void emit_armz(int offset);
void emit_soma(void);
void emit_subt(void);
void emit_mult(void);
void emit_divi(void);
void emit_invr(void);
void emit_conj(void);
void emit_disj(void);
void emit_cmme(void);
void emit_cmma(void);
void emit_cmeq(void);
void emit_cdifeq(void);
void emit_cmmeq(void);
void emit_cmmaq(void);
void emit_dsvs(const char *label);
void emit_dsvf(const char *label);
void emit_chpr(void);
void emit_chle(void);
void emit_para(int n);
void emit_cham(const char *label);
void emit_retorno(void);
void emit_label(const char *label);
char* new_label(void);

// Instruções para vetores
void emit_crvi(void);   // carrega do endereço no topo
void emit_armi(void);   // armazena no endereço no topo
void emit_troca(void);  // troca os dois topo da pilha
void emit_chpd(void);

#endif
