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

int last_status = 0;
int builtin_executed = 0;
char home_dir[256];

// https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_CYAN    "\033[1;36m"

void smart_prompt(int laststatus)
{
    char path[256];

    struct new_utsname uts;

    long ret = sys_getcwd(path, sizeof(path));
    if (ret < 0) {
        path[0] = '!';
        path[1] = '\0';
    }

    sys_newuname(&uts);

    const char *color = (last_status == 0) ? COLOR_GREEN : COLOR_RED;
    const char *symbol = (last_status == 0) ? "🟢" : "🔴";

    my_printf("%smysh%s [%d] %s%s%s %s%s%s %s ",
              COLOR_CYAN, COLOR_RESET,
              last_status,
              color, uts.nodename, COLOR_RESET,
              COLOR_YELLOW, path, COLOR_RESET,
              symbol);
}

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
    const char *cd = "cd";
    const char *exit = "exit";
    const char *history = "history";

    install_sigchld();
    ldisc_init();
    heap_init();
    map_init(&symbol_table, 10);
    symtab_build_from_env(&symbol_table, envp);
    symtab_get_home_directory(home_dir, sizeof(home_dir), envp);
    map_insert(&symbol_table, cd, NULL, 1);
    map_insert(&symbol_table, exit, NULL, 1);
    map_insert(&symbol_table, history, NULL, 1);
    lexer_init();

    init_history();

    while (1) {
        const char * line;
        line = my_readline(last_status);
        if (!line) continue;
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
        int status = -1;

        if (pid > 0) {
            sys_wait4(pid, &status, 0, NULL);
        }

        if (builtin_executed) {
            builtin_executed = 0;
            last_status = 0;
        } else {
            last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
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