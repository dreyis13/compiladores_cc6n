/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef LOG_H
#define LOG_H

#include "lex.h"

/* Funções para gerenciar o arquivo de lista de tokens (.tk) */
void log_open_tokens(const char *base_filename);
void log_write_token(Token tok);
void log_close_tokens(void);

/* Funções para gerenciar o arquivo de rastreamento (.trc) */
void log_open_trace(const char *base_filename);
void log_write_trace(const char *msg);
void log_close_trace(void);

#endif
