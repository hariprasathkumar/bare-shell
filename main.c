#include "ldisc.h"
#include "history.h"
#include "heap.h"
#include "map.h"
#include "symtab.h"

int main(int argc, char *argv[], char *envp[])
{
    struct hash *symbol_table;

    ldisc_init();
    heap_init();
    map_init(&symbol_table, 10);
    symtab_build_from_env(&symbol_table, envp);

    init_history();

    while (1) {
        const char * line;
        line = my_readline("mysh$ ");
        if (!line) break;
        update_history(line);
    }

    ldisc_deinit();
    free_history();
    free_map(symbol_table);
    
    return 0;
}