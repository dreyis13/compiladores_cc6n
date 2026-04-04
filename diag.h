/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#ifndef DIAG_H
#define DIAG_H

void diag_init(int trace_enabled);
void diag_info(const char *format, ...);
void diag_error(int line, const char *expected, const char *found);
int diag_error_count(void);
void diag_cleanup(void);

#endif
