
#ifndef _PARSER_H
#define _PARSER_H

#include "machine.h"
#include "lexer.h"

enum redir_type {
     REDIR_OUT,
     REDIR_IN,
     REDIR_APPEND,
};

struct redirection {
   enum redir_type type;
   char *filename;
   struct redirection *next;
};

struct command {
   int argc;
   char **argv;
   struct redirection *redir;
};

struct pipeline {
   struct ast *left;
   struct ast *right;
};

struct list {
   struct ast *left;
   struct ast *right;
   char op[2];
};

enum ast_type {
   AST_COMMAND,
   AST_PIPELINE,
   AST_LIST,
}; 

struct ast {
    enum ast_type type;
    union {
        struct command cmd;
        struct pipeline pipe;
        struct list list;
    } u;
};

struct ast *parse(const struct token_stream *tokens);
struct ast *parse_list(const struct token_stream *ts, size_t *pos);
struct ast *parse_pipeline(const struct token_stream *ts, size_t *pos);
struct ast *parse_command(const struct token_stream *ts, size_t *pos);
struct redirection *parse_redirection(const struct token_stream *ts, size_t *pos);

void free_ast(struct ast *node);
void free_redirections(struct redirection *r);
void print_ast(struct ast *root, int depth);

#endif