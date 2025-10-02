
#ifndef _LEXER_H
#define _LEXER_H

#include "machine.h"

enum token_type {
    TOK_SEMICOLON, // ;       
    TOK_AMP,   // &      
    TOK_AND_IF,// &&  
    TOK_PIPE,  // | 
    TOK_OR_IF, // ||      
    TOK_REDIRECT_OUT, // >   
    TOK_REDIRECT_OUT_APPEND, // >>
    TOK_REDIRECT_IN,  // <
    TOK_WORD,   // [a-zA-Z0-9._/-]+     
    TOK_END          
};

struct token {
    enum token_type type;
    char *lexeme;
};

struct token_stream {
    struct token *arr;
    size_t count;
    size_t capacity;
};

void lexer_init(void);
const struct token_stream *tokenize(const char *line, size_t *ntokens);
void free_tokens(void);
void lexer_deinit(void);

// debug
void print_tokens(void);
#endif
