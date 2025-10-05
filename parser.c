
#include "parser.h"
#include "print.h"
#include "heap.h"
#include "memory.h"

// Get current token without consuming
enum token_type peek_token(const struct token_stream *ts, size_t pos)
{
    if (pos >= ts->count) return TOK_END;

    return ts->arr[pos].type;
}

// Consume current token and move forward
struct token *next_token(const struct token_stream *ts, size_t *pos)
{
    if (*pos >= ts->count) return NULL;

    return &ts->arr[(*pos)++];
}

static void parse_error(const char *msg, struct token *t)
{
    my_printf("msg: %s, token seen: %d", msg, (int)t->type);
}

static struct ast *create_ast_node(void)
{
    void *ret = my_malloc(sizeof(struct ast));
    if (!ret) return NULL;

    my_memset(ret, 0, sizeof(struct ast));

    return (struct ast *)ret;
}

static struct redirection *create_redirect_node(void)
{
    void *ret = my_malloc(sizeof(struct redirection));
    if (!ret) return NULL;

    my_memset(ret, 0, sizeof(struct redirection));

    return (struct redirection *) ret;
}

static struct ast **alloc_cmd_array(size_t size)
{
    struct ast **ret = (struct ast **)my_malloc(sizeof(struct ast *) * size);
    if (!ret) return NULL;

    my_memset(ret, 0, sizeof(struct ast *) * size);

    return (struct ast **)ret;
}

static struct ast **resize_cmd_array(struct ast **cur, size_t old_size, size_t size)
{
    struct ast **new = alloc_cmd_array(size);
    if (!new) return NULL;

    my_memset(new, 0, sizeof(struct ast *) *size);
    my_memcpy(new, cur, sizeof(struct ast *) *old_size);

    my_free(cur);

    return new;
}

static char **alloc_string_array(size_t size)
{
    size++; // null terminator
    char **ret = (char **)my_malloc(sizeof(char *) * size);
    if (!ret) return NULL;

    my_memset(ret, 0, sizeof(char *) * size);

    return ret;
}

static char **resize_string_array(char **cur, size_t size)
{
    // ensure size for this call is always bigger
    char **ret = alloc_string_array(size);
    char **i = ret;
    char **orginal = cur;

    if (!ret) return NULL;

    while (*cur) {
        *i = *cur;
        i++;
        cur++;
    }

    my_free(orginal);

    return ret;
}

static char **get_words(const struct token_stream *ts, size_t *pos, int *argc)
{
    size_t arr_size = 10, i = 0;
    char **ret = alloc_string_array(arr_size);
    enum token_type type = peek_token(ts, *pos);

    if (!ret) return NULL;

    while (type == TOK_WORD) {
        if (i == arr_size) {
            arr_size *= 2;
            ret = resize_string_array(ret, arr_size);
            if (!ret) return NULL;
        }

        struct token *c = next_token(ts, pos);
        ret[i++] = c->lexeme;
        type = peek_token(ts, *pos);
    }
    
    *argc = i;

    return ret;
}

struct ast *parse(const struct token_stream *tokens)
{
    size_t pos = 0;

    return parse_list(tokens, &pos);
}

// list          = pipeline { (";" | "&" | "&&" | "||") pipeline } ;
struct ast *parse_list(const struct token_stream *ts, size_t *pos)
{
    struct ast *node = NULL; 
    node = parse_pipeline(ts, pos);

    enum token_type type = peek_token(ts, *pos);

    // 0 or more occurence
    while (type == TOK_SEMICOLON || type == TOK_AMP || type == TOK_AND_IF || type == TOK_OR_IF)
    {
        struct ast *new = create_ast_node();
        if (!new) return node;

        new->type = AST_LIST;
        new->u.list.left = node;

        (void)next_token(ts, pos);

        if (type == TOK_SEMICOLON) {
            new->u.list.op[0] = ';';
            new->u.list.op[1] = ' ';
        } else if (type == TOK_AMP) {
            new->u.list.op[0] = '&';
            new->u.list.op[1] = ' ';
        } else if (type == TOK_AND_IF) {
            new->u.list.op[0] = '&';
            new->u.list.op[1] = '&';
        } else {
            new->u.list.op[0] = '|';
            new->u.list.op[1] = '|';
        }

        enum token_type iseol = peek_token(ts, *pos) == TOK_END ? 1 : 0;
        if (!iseol)
        {
            new->u.list.right = parse_pipeline(ts, pos);
        }
        else
        {
            new->u.list.right = NULL;
        }

        node = new;

        type = peek_token(ts, *pos);
    }

    return node;
}

// pipeline      = command { "|" command } 
struct ast *parse_pipeline(const struct token_stream *ts, size_t *pos)
{
    int size = 10;
    int count = 0;
    struct ast *node = create_ast_node();
    struct ast **cmd = alloc_cmd_array(size);
    if (!node) return NULL;

    node->type = AST_PIPELINE;
    cmd[count++] = parse_command(ts, pos);

    enum token_type type = peek_token(ts, *pos);

    while (type == TOK_PIPE)
    {
        if (count == size)
        {
            cmd = resize_cmd_array(cmd, size, size*2);
            if (!cmd) return NULL;

            size *= 2;
        }

        (void)next_token(ts, pos);

        cmd[count++] = parse_command(ts, pos);
        type = peek_token(ts, *pos);
    }

    node->u.pipe.cmds = cmd;
    node->u.pipe.count = count;

    return node;
}

// command       = word { word } { redirection } ;
struct ast *parse_command(const struct token_stream *ts, size_t *pos)
{
    struct ast *node = create_ast_node();
    int argc;

    if (!node) return NULL;

    node->type = AST_COMMAND;
    node->u.cmd.argv = get_words(ts, pos, &argc);
    node->u.cmd.argc = argc;

    enum token_type type = peek_token(ts, *pos);

    if (type == TOK_REDIRECT_OUT || type == TOK_REDIRECT_OUT_APPEND || type == TOK_REDIRECT_IN)
    {
        node->u.cmd.redir = parse_redirection(ts, pos);
    }
    else 
    {
        node->u.cmd.redir = NULL;
    }

    return node;
}

// redirection   = ( ">" word 
//                 | ">>" word 
//                 | "<" word ) ;

struct redirection *parse_redirection(const struct token_stream *ts, size_t *pos)
{
    struct redirection *list = NULL;
    enum token_type type = peek_token(ts, *pos);

    switch (type)
    {
        case TOK_REDIRECT_OUT:
        {
            (void)next_token(ts, pos);
            
            struct token *t = next_token(ts, pos);

            list = create_redirect_node();
            if (!list) return NULL;

            list->type = REDIR_OUT;
            list->filename = t->lexeme;
            list->next = parse_redirection(ts, pos);
            break;
        }

        case TOK_REDIRECT_OUT_APPEND:
        {
            (void)next_token(ts, pos);
            
            struct token *t = next_token(ts, pos);

            list = create_redirect_node();
            if (!list) return NULL;

            list->type = REDIR_APPEND;
            list->filename = t->lexeme;
            list->next = parse_redirection(ts, pos);
            break;
        }

        case TOK_REDIRECT_IN:
        {
            (void)next_token(ts, pos);
            
            struct token *t = next_token(ts, pos);

            list = create_redirect_node();
            if (!list) return NULL;

            list->type = REDIR_IN;
            list->filename = t->lexeme;
            list->next = parse_redirection(ts, pos);
            break;
        }
    }

    return list;
}

void free_redirections(struct redirection *r)
{
    if (!r) return;

    free_redirections(r->next);
    my_free(r);
}

void free_ast(struct ast *node)
{
    if (!node) return;

    switch (node->type) {
        case AST_COMMAND:
        {
            my_free(node->u.cmd.argv);
            free_redirections(node->u.cmd.redir);
            break;
        }

        case AST_PIPELINE: 
        {
            size_t c = node->u.pipe.count;
            for (size_t i = 0; i < c; i++) {
                free_ast(node->u.pipe.cmds[i]);
            }
            my_free(node->u.pipe.cmds);
            break;
        }

        case AST_LIST:
        {
            free_ast(node->u.list.left);
            free_ast(node->u.list.right);
            break;
        }
    }

    my_free(node);
}

static void print_pretty(int depth)
{
    const char *space = "  ";

    while(depth--){
        my_printf("%s", space);
    }
}

void print_ast(struct ast *root, int depth)
{
    switch (root->type)
    {
        case AST_COMMAND:
        {
            print_pretty(depth);
            my_printf("AST_COMMAND:\n");
            size_t c = 0;
            for ( ; c < root->u.cmd.argc; c++ )
            {
                print_pretty(depth);
                my_printf("  arg %d = %s\n", c, root->u.cmd.argv[c]);
            }

            struct redirection *r = root->u.cmd.redir;
            while (r) {
                print_pretty(depth);
                switch (r->type)
                {
                    case REDIR_IN:
                    {
                        my_printf("  redirect type: REDIR_IN %s\n", r->filename);
                        break;
                    }

                    case REDIR_OUT:
                    {
                        my_printf("  redirect type: REDIR_OUT %s\n", r->filename);
                        break;
                    }

                    case REDIR_APPEND:
                    {
                        my_printf("  redirect type: REDIR_APPEND %s\n",  r->filename);
                        break;
                    }
                }

                r = r->next;
            }
            break;
        }

        case AST_PIPELINE:
        {
            print_pretty(depth);
            my_printf("AST_PIPELINE:\n");
            size_t i = 0, c = root->u.pipe.count;
            for ( ; i < c; i++)
            {
                print_ast(root->u.pipe.cmds[i], depth+1);
            }
            break;
        }

        case AST_LIST:
        {
            print_pretty(depth);
            my_printf("AST_LIST\n");
            print_pretty(depth);
            my_printf("  AST_LIST: op[0] = %c op[1] = %c\n", root->u.list.op[0], root->u.list.op[1]);
            if (root->u.list.left)
            print_ast(root->u.list.left, depth+1);
            if (root->u.list.right)
            print_ast(root->u.list.right, depth+1);
            break;
        }
    }
}
