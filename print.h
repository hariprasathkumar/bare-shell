
#ifndef _PRINT_H
#define _PRINT_H

#include "machine.h"
#include <stdarg.h> // not part of libc

int my_vsnprintf(char *str, size_t size, const char *format, va_list ap);
int my_printf(const char* format, ...);

#endif
