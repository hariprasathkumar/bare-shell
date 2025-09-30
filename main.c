#include "ldisc.h"
#include "history.h"
#include "heap.h"

int main()
{
    ldisc_init();
    heap_init();
    init_history();

    while (1) {
        const char * line;
        line = my_readline("mysh$ ");
        if (!line) break;
        update_history(line);
    }

    ldisc_deinit();
    free_history();
    
    return 0;
}