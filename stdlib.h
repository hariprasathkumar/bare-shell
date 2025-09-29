
#ifndef _STDLIB_H
#define _STDLIB_H

long int my_strtol(const char *restrict nptr, char **restrict endptr, int base);
int my_atoi(const char *nptr);
char *my_itoa(int value, char *str, int base);

#endif