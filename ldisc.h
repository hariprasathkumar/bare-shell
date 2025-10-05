
#ifndef _LDISC_H
#define _LDISC_H

#include "machine.h"

void   ldisc_init(void);
void ldisc_deinit(void);
size_t ldisc_get_line(char *line, size_t line_length);
const char *my_readline(int last_status);

#endif