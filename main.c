#include "ldisc.h"
#include "history.h"

int main()
{
    ldisc_init();
    init_history();

    while (1) {
        const char * line;
        line = my_readline("mysh$ ");
        if (!line) break;
        update_history(line);
    }

    ldisc_deinit();
}