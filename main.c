#include "ldisc.h"
#include "history.h"
#include "heap.h"
#include "map.h"
#include "symtab.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[], char *envp[])
{
    struct hash *symbol_table;

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

        free_ast(tree);
        free_tokens();
    }

    ldisc_deinit();
    lexer_deinit();
    free_history();
    free_map(symbol_table);
    
    return 0;
}