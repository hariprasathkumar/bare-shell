
#ifndef _EXECUTE_H
#define _EXECUTE_H

#include "syscall.h"
#include "parser.h"
#include "map.h"

#define STDIN_FD    0
#define STDOUT_FD   1

pid_t execute_ast(const struct ast *tree, const char * const *envp, struct hash **map, int infd, int outfd);
pid_t execute_list(const struct list *list,  const char * const *envp, struct hash **map, int infd, int outfd);
pid_t execute_pipeline(const struct pipeline *pipe, const char * const *envp, struct hash **map, int infd, int outfd);
pid_t execute_command(const struct ast *t, const char * const *envp, struct hash **map, int infd, int outfd);

#endif