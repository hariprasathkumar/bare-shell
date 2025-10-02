
#ifndef _SYMTAB_H
#define _SYMTAB_H

#include "map.h"

#define AT_FDCWD		-100 
#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002
#define O_DIRECTORY	00200000	

#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4


long symtab_build_from_env(struct hash **symtab, char **envp);

#endif