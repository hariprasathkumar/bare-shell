
#ifndef _SYMTAB_H
#define _SYMTAB_H

#include "map.h"

long symtab_get_home_directory(char *home, size_t home_len, char **envp);
long symtab_build_from_env(struct hash **symtab, char **envp);

#endif