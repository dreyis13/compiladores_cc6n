/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef DIAG_H
#define DIAG_H

/* Inicializa o sistema de diagnóstico (habilita ou não o trace) */
void diag_init(int trace_enabled);

/* Exibe mensagem informativa (se trace ativado) e grava no arquivo .trc */
void diag_info(const char *format, ...);

/* Registra um erro na linha especificada, com o token esperado e o encontrado */
void diag_error(int line, const char *expected, const char *found);

/* Retorna o número total de erros registrados */
int diag_error_count(void);

/* Libera recursos (função vazia, mantida por consistência) */
void diag_cleanup(void);

#endif
