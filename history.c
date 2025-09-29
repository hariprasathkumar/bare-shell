#include "history.h"
#include "memory.h"
#include "print.h"
#include "string.h"

#define MAX_HISTORY 1024
#define CMD_BUF_SIZE 512

struct hist_entry {
    size_t num;
    char arr[CMD_BUF_SIZE];
};

static struct hist_entry history[MAX_HISTORY];
static size_t next_num = 1;
static size_t head = 0;
static size_t count = 0;
static size_t cursor = 0;

void init_history(void)
{
    for (size_t i = 0; i < MAX_HISTORY; i++) 
    {
        my_memset(history[i].arr, 0, CMD_BUF_SIZE);
        history[i].num = 0;
    }
    next_num = 1;
    head = 0;
    count = 0;
    cursor = 0;
}

const char *get_history(size_t cmd)    
{
    size_t start = (count < MAX_HISTORY) ? 0: head;
    size_t limit = count;

    while (limit--) {
        if (history[start].num == cmd) {
            return history[start].arr;
        }
        start = (start + 1) % MAX_HISTORY;
    }
    return NULL;
}

void update_history(const char *cmd)
{
    if (!cmd) return;

    size_t last_cmd = (head + MAX_HISTORY - 1) % MAX_HISTORY;

    if (my_strncmp(cmd, history[last_cmd].arr, CMD_BUF_SIZE) == 0)
    {
        return; // dont add
    }
    my_strncpy(history[head].arr, cmd, CMD_BUF_SIZE);
    history[head].num = next_num;

    next_num++;
    head = (head + 1) % MAX_HISTORY;
    if (count < MAX_HISTORY) count++;
    cursor = head;
}

const char *get_previous_history(void) // up arrow
{
    size_t oldest = (head + MAX_HISTORY - count) % MAX_HISTORY;

    if (cursor != oldest) {
        cursor = (cursor + MAX_HISTORY - 1) % MAX_HISTORY;
    }

    return history[cursor].arr;
}

const char *get_next_history(void) // down arrow
{
    size_t latest = (head + MAX_HISTORY - 1) % MAX_HISTORY;

    if (cursor != latest) {
        cursor = (cursor + 1) % MAX_HISTORY;
    }

    return history[cursor].arr;
}

void print_history(void)
{
    size_t oldest = (head + MAX_HISTORY - count) % MAX_HISTORY;
    size_t limit = count;
    
    while (limit--) {
        my_printf("%d %s\n", history[oldest].num, history[oldest].arr);
        oldest = (oldest + 1) % MAX_HISTORY;
    }
}
