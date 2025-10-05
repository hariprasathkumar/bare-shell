#include "ldisc.h"
#include "history.h"
#include "heap.h"
#include "map.h"
#include "symtab.h"
#include "lexer.h"
#include "parser.h"
#include "execute.h"
#include "syscall.h"
#include "print.h"
#include "memory.h"

void sigchld_handler(int signo)
{
    int status;
    pid_t pid;

    while ((pid = sys_wait4(-1, &status, WNOHANG, 0)) > 0){
        //my_printf("pid %d exited\n", (long)pid);
    }
}

void install_sigchld(void)
{
    struct sigaction act;
    my_memset(&act, 0, sizeof(act));

    act.sa_handler  = sigchld_handler;
    act.sa_flags    = SA_RESTART | SA_NOCLDSTOP | SA_RESTORER;
    act.sa_restorer = sys_rt_sigreturn;

    long ret = sys_rt_sigaction(SIGCHLD, &act, NULL, 8);

    //my_printf("sizeof(struct sigaction)=%d sigset_size=128 ret=%d\n",
    //          (int)sizeof(act), ret);
}

int main(int argc, char *argv[], char *envp[]);

__attribute__((naked)) void _start(void)
{
    long *stack;
    
    asm ("mov %%rsp, %0" : "=r" (stack));

    int argc = *stack;
    char **argv = (char **)stack;
    
    while (*stack) stack++;
    stack++;

    char **envp = (char **) stack;

    int ret = main(argc, argv, envp);

    sys_exit(ret);
}

int main(int argc, char *argv[], char *envp[])
{
    struct hash *symbol_table;

    install_sigchld();
    ldisc_init();
    heap_init();
    map_init(&symbol_table, 10);
    symtab_build_from_env(&symbol_table, envp);
    lexer_init();

    init_history();

    while (1) {
        const char * line;
        line = my_readline("mysh$ ");
        if (!line) break;
        update_history(line);

        size_t ntokens = 0;
        const struct token_stream *ts = tokenize(line, &ntokens);

        if (!ts) {
            break;
        }

        struct ast *tree = parse(ts);

        if (!tree) {
            free_tokens();
            break;
        }

        ldisc_deinit();
        pid_t pid = execute_ast(tree, (const char * const*)envp, &symbol_table, 0, 1);
        int status;

        if (pid > 0) {
            sys_wait4(pid, &status, 0, NULL);
        }

        ldisc_init();
        free_ast(tree);
        free_tokens();
    }

    ldisc_deinit();
    lexer_deinit();
    free_history();
    free_map(symbol_table);
    
    return 0;
}