
#ifndef _HISTORY_H
#define _HISTORY_H

#include "machine.h"

void init_history(void);
void print_history(void);
const char *get_history(size_t cmd)  ;
void  update_history(const char *cmd);
const char *get_previous_history(void);
const char *get_next_history(void);

#endif