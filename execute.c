
#include "execute.h"
#include "parser.h"
#include "map.h"


static void close_fd_wrapper(int fd)
{
    if (fd >= 0) sys_close(fd);
}

/*
struct redirection {
   enum redir_type type;
   char *filename;
   struct redirection *next;
};
*/
static int handle_redirection(struct redirection *redir)
{

    while (redir)
    {
        switch (redir->type)
        {
            case REDIR_OUT:
            {
                int newfd = sys_openat(AT_FDCWD, redir->filename, O_WRONLY | O_CREAT | O_TRUNC , 0644);
                if (newfd < 0) {
                    //my_printf("handle_redirection, redir_out\n");
                    return -1;
                }

                long ret = sys_dup2(newfd, STDOUT_FD);
                if (ret < 0) {
                    //my_printf("handle_redirection, sys_dup2 redir_out\n");
                    return -1;
                }

                close_fd_wrapper(newfd);
                break;
            }

            case REDIR_IN:
            {
                int newfd = sys_openat(AT_FDCWD, redir->filename, O_RDONLY , 0);
                if (newfd < 0)  {
                    //my_printf("handle_redirection, sys_openat REDIR_IN\n");
                    return -1;
                }

                long ret = sys_dup2(newfd, STDIN_FD);
                if (ret < 0)  {
                    //my_printf("handle_redirection, sys_dup2 REDIR_IN\n");
                    return -1;
                }

                close_fd_wrapper(newfd);
                break;
            }

            case REDIR_APPEND:
            {
                int newfd = sys_openat(AT_FDCWD, redir->filename, O_WRONLY | O_CREAT | O_APPEND , 0644);
                if (newfd < 0)  {
                    //my_printf("handle_redirection, sys_openat REDIR_APPEND\n");
                    return -1;
                }

                long ret = sys_dup2(newfd, STDOUT_FD);
                if (ret < 0) {
                    //my_printf("handle_redirection, sys_dup2 REDIR_APPEND\n");
                    return -1;
                }

                close_fd_wrapper(newfd);
                break;
            }
        }

        redir = redir->next;
    }

    return 0;
}


/*
struct ast {
    enum ast_type type;
    union {
        struct command cmd;
        struct pipeline pipe;
        struct list list;
    } u;
}
*/
pid_t execute_ast(const struct ast *tree, const char * const *envp, struct hash **map, int infd, int outfd)
{
    if (!tree) return -1;

    pid_t pid;

    switch (tree->type)
    {
        case AST_COMMAND:
        {
            pid = execute_command(tree, envp, map, infd, outfd);
            break;
        }

        case AST_PIPELINE:
        {
            pid = execute_pipeline(&tree->u.pipe, envp, map, infd, outfd);
            //my_printf("g %d\n", tree->u.pipe.count);
            break;
        }

        case AST_LIST:
        {
            pid = execute_list(&tree->u.list, envp, map, infd, outfd);
            break;
        }
    }

    return pid;
}

/*
struct list {
   struct ast *left;
   struct ast *right;
   char op[2];
   char background;
};
*/
//list          = pipeline { (";" | "&" | "&&" | "||") pipeline } ;
pid_t execute_list(const struct list *list, const char * const *envp, struct hash **map, int infd, int outfd)
{
    pid_t pid = -1;
    pid_t child;
    int status_of_previous_command;

    //my_printf("g %d %d\n", list->left->type, list->left->u.pipe.count);
    pid = execute_ast(list->left, envp, map, infd, outfd);

    if (list->op[0] == ';' && list->op[1] == ' ')
    {
        child = sys_wait4(pid, &status_of_previous_command, 0, NULL);
    }
    else if (list->op[0] == '&' && list->op[1] == ' ')
    {
        // background
    }
    else if ((list->op[0] == '&' && list->op[1] == '&') || (list->op[0] == '|' && list->op[1] == '|'))
    {
        child = sys_wait4(pid, &status_of_previous_command, 0, NULL);

        int ok = WIFEXITED(status_of_previous_command) && (WEXITSTATUS(status_of_previous_command) == 0);
        if ((list->op[0] == '&' && list->op[1] == '&' && !ok) ||
            (list->op[0] == '|' && list->op[1] == '|' && ok)) 
        {
            return pid;
        }
    }

    if (list->right) {
        pid = execute_ast(list->right, envp, map, infd, outfd);
    }

    while (sys_wait4(-1, &status_of_previous_command, WNOHANG, NULL) > 0); // wait for all child to exit

    return pid;
}

/*
struct pipeline {
   int count;
   struct ast **cmds;
};
*/
//pipeline      = command { "|" command } ;
pid_t execute_pipeline(const struct pipeline *pipe, const char * const *envp, struct hash **map, int infd, int outfd)
{
    pid_t pid = 0;
    long ret = 0;
    int cur_out, cur_read = infd;

    // chain commands
    for ( size_t i = 0; i < pipe->count; i++ )
    {
        int fd[2] = {-1, -1}; // 0 - read, 1 - write

        if (i != pipe->count - 1) 
        {
            if (sys_pipe2(fd, 0) < 0) {
                //my_printf("execute_pipeline, sys_pipe2 \n");
                return -1;
            }
            cur_out = fd[1];
        } else {
            cur_out = outfd;
        }

        //my_printf("%d\n", pipe->count);
        //my_printf("%d\n", pipe->cmds[i]);
        pid = execute_command(pipe->cmds[i], envp, map, cur_read, cur_out);
        if (pid < 0) {
            if (i != pipe->count - 1) {
                close_fd_wrapper(fd[0]);
                close_fd_wrapper(fd[1]);
            }
            return -1;
        }
        
        if (i != pipe->count - 1)
        {
            close_fd_wrapper(fd[1]);
            if (cur_read != infd) close_fd_wrapper(cur_read);
            cur_read = fd[0]; // copy previous pipe read fd
        }
    }

    if (cur_read != infd) close_fd_wrapper(cur_read);

    return pid;
}

/*
struct command {
   int argc;
   char **argv;
   struct redirection *redir;
};
*/
//command       = word { word } { redirection } ;
pid_t execute_command(const struct ast *t, const char * const *envp, struct hash **map, int infd, int outfd)
{
    const struct command *cmd = &t->u.cmd;
    const char *cmd_name = cmd->argv[0];
    long ret, pid;
    struct sym_entry *s = map_lookup(map, cmd_name);

    if (!s) {
        //my_printf("execute_command, map_lookup \n");
        return -1;
    }

    pid = sys_fork();
    if (pid < 0) {
        //my_printf("execute_command, sys_fork \n");
        return -1;
    }

    if (pid == 0)
    {
        if (infd != STDIN_FD)
        {
            ret = sys_dup2(infd, STDIN_FD);
            if (ret < 0) {
                //my_printf("execute_command, sys_dup2 1 \n");
                return -1;
            }
            close_fd_wrapper(infd);
        }

        if (outfd != STDOUT_FD)
        {
            ret = sys_dup2(outfd, STDOUT_FD);
            if (ret < 0) {
                //my_printf("execute_command, sys_dup2 2 \n");
                return -1;
            }
            close_fd_wrapper(outfd);   
        }

        if (cmd->redir)
        {
            if (handle_redirection(cmd->redir) < 0) return -1;
        }

        ret = sys_execve(s->path, (const char * const *)cmd->argv, envp);
        if (ret < 0) sys_exit(127);
    }

    return pid;
}