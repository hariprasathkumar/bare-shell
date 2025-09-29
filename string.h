
#ifndef _STRING_H
#define _STRING_H

#include "machine.h"

size_t my_strlen(const char *s);
int my_strcmp(const char *s1, const char *s2);
int my_strncmp(const char *s1, const char *s2, size_t n);
char *my_strncpy(char *restrict dest, const char *restrict src, size_t n);

#endif 