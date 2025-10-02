
#include "lexer.h"
#include "heap.h"
#include "memory.h"
#include "string.h"
#include "print.h"

#define MAX_TOKENS  512

static struct token_stream tokens;

void lexer_init(void) 
{
    struct token *t = (struct token *)my_malloc(sizeof(struct token) * MAX_TOKENS);
    if (!t) 
    {
        my_printf("token stream alloc failed\n");
        return ;
    }

    my_memset(t, 0, sizeof(struct token) * MAX_TOKENS);

    tokens.arr = t;
    tokens.capacity = MAX_TOKENS;
    tokens.count = 0;
}

static void update_word(char *buf, size_t *i)
{
    if (*i != 0) {
        buf[*i] = '\0';
        *i = 0;
        tokens.arr[tokens.count].type = TOK_WORD;
        tokens.arr[tokens.count].lexeme =  my_strdup(buf);
        tokens.count++;
    }
}

// no realloc so
static void realloc_tokens(void) 
{
    struct token *a = (struct token *)my_malloc(sizeof(struct token)*tokens.capacity*2);
    if (!a) {
        my_printf("realloc tokens failed\n");
        return;
    }

    for (size_t i = 0; i < tokens.count; i++) {
        a[i].type = tokens.arr[i].type;
        if (tokens.arr[i].lexeme) {
            a[i].lexeme = tokens.arr[i].lexeme;
        } 
    }

    my_free(tokens.arr);
    tokens.arr = a;
    tokens.capacity *= 2;
} 

const struct token_stream *tokenize(const char *line, size_t *ntokens)
{
    char *buf = my_malloc(258);
    size_t i = 0;

    while (*line && i < 256) {
        if (tokens.count == tokens.capacity) {
            realloc_tokens();
        }

        if (*line == ' ') {
            update_word(buf, &i);
            line++;
        } else if (*line == ';') {
            update_word(buf, &i);
            tokens.arr[tokens.count].type = TOK_SEMICOLON;
            tokens.arr[tokens.count].lexeme = NULL;
            i = 0; 
            line++;
            tokens.count++;
        } else if (*line == '&') {
            update_word(buf, &i);
            if (*(line + 1) == '&') {
                tokens.arr[tokens.count].type = TOK_AND_IF;
                line++;
            } else {
                tokens.arr[tokens.count].type = TOK_AMP;
            }
            tokens.arr[tokens.count].lexeme = NULL;
            i = 0; 
            line++;
            tokens.count++;
        } else if (*line == '|') {
            update_word(buf, &i);
            if ( *(line + 1) == '|') {
                tokens.arr[tokens.count].type = TOK_OR_IF;
                line++;
            } else {
                tokens.arr[tokens.count].type = TOK_PIPE;
            }
            tokens.arr[tokens.count].lexeme = NULL;
            i = 0;
            line++;
            tokens.count++;
        } else if (*line == '>') {
            update_word(buf, &i);
            if ( *(line + 1) == '>') {
                tokens.arr[tokens.count].type = TOK_REDIRECT_OUT_APPEND;
                line++;
            } else {
                tokens.arr[tokens.count].type = TOK_REDIRECT_OUT;
            }
            tokens.arr[tokens.count].lexeme = NULL;
            i = 0;
            line++;
            tokens.count++;
        } else if (*line == '<') {
            update_word(buf, &i);
            tokens.arr[tokens.count].type = TOK_REDIRECT_IN;
            tokens.arr[tokens.count].lexeme = NULL;
            i = 0;
            line++;
            tokens.count++;
        } else if (*line == '\n') { // sentinel
            update_word(buf, &i);
            tokens.arr[tokens.count].type = TOK_END;
            tokens.arr[tokens.count].lexeme = NULL;
            i = 0;
            line++;
            tokens.count++;
            break;
        } else {
            buf[i++] = *line;
            line++;
        }
    }

    my_free(buf);

    if (i == 256) {
        *ntokens = 0;
        return NULL;
    }

    *ntokens = tokens.count;
    return &tokens;
}

void free_tokens(void)
{
    size_t cap = tokens.count;

    for (size_t i = 0; i < cap; i++) {
        if (tokens.arr[i].lexeme)
        {
            my_free(tokens.arr[i].lexeme);
            tokens.arr[i].lexeme = NULL;
        }
        tokens.arr[i].type = 0;
    }

    tokens.count = 0;
}

void lexer_deinit(void)
{
    if (tokens.arr) {
        my_free(tokens.arr);
    }
}

void print_tokens(void) 
{
    size_t cap = tokens.count;

    for (size_t i = 0; i < cap; i++)
    {
        my_printf("token type %d, %s", (int)tokens.arr[i].type, tokens.arr[i].lexeme);
    }
} 
