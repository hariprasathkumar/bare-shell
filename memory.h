
#ifndef _MEMORY_H
#define _MEMORY_H

#include "machine.h"

void *my_memset(void *s, int c, size_t n);
void *my_memcpy(void *restrict dest, const void *restrict src, size_t n);
void *my_memmove(void *dest, const void *src, size_t n);
int my_memcmp(const void *s1, const void *s2, size_t n);

#endif