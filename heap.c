
#include "heap.h"
#include "syscall.h"
#include "print.h"

struct block {
    size_t magic;
    size_t size;
    int free;
    struct block *next;
    struct block *prev;
};

static struct block *heap_start;
static void *heap_end;

static size_t align(size_t size)
{
    return (size + MACHINE_ALIGNMENT - 1) & ~(MACHINE_ALIGNMENT - 1);
}

void walk(void)
{
    struct block *s = heap_start;

    while (s) {
        my_printf("size: %d free: %d\n", s->size, s->free);
        s = s->next;
    }
}

static void split_block(struct block *blk, size_t size)
{
    size_t total = size + sizeof(struct block);
    struct block *next = (struct block *)(total + (char *)blk);

    next->magic = MAGIC_HEADER;
    next->size = blk->size - total;
    next->free = 1;
    next->next = blk->next;
    next->prev = blk;
    blk->next = next;
    blk->size = size;
}

static struct block *extend_heap(size_t siz)
{
    size_t size = align(siz);
    void *last_end = heap_end;
    long ret = sys_brk((unsigned long)((char *)heap_end + size));
    if (ret < 0) {
        my_printf("Heap extension failed\n");
        return NULL;
    }

    struct block *s = heap_start;
    heap_end = (void *)ret;

    while (s->next) s = s->next;
    
    if (s->free) {
        s->size += size;
    } else {
        struct block *new_blk = (struct block *)last_end;
        new_blk->magic = MAGIC_HEADER;
        new_blk->size  = size - sizeof(struct block);
        new_blk->free  = 1;
        new_blk->next  = NULL;
        new_blk->prev  = s;
        s->next = new_blk;
    }

    return heap_start;
}

static struct block *find_free(size_t size)
{
    struct block *s = heap_start;
    size_t aligned_size = align(size);

    while (s) {
        if (s->free && s->size >= aligned_size) {
            break;
        }
        s = s->next;
    }

    if (!s) return NULL;

    if (s->size > aligned_size + sizeof(struct block))
    {
        split_block(s, aligned_size);
    }
    s->free = 0;

    return s;
}

static void coalesce(struct block *blk)
{
    struct block *c = blk;

    if (c->prev && c->prev->free) {
        struct block *p = c->prev;
        p->size += c->size + sizeof(struct block);
        c->magic = 0;
        p->next = c->next;
        if (c->next) {
            c->next->prev = p;
        }
        c = p;
    }

    struct block *next = c->next;
    if (next && next->free)
    {
        c->size += next->size + sizeof(struct block);
        next->magic = 0;
        c->next = next->next;
        if (c->next) {
            c->next->prev = c;
        }
    }
}

long heap_init() 
{
    heap_start = (struct block *)sys_brk(0);
    long ret = sys_brk((unsigned long)((char *)heap_start + HEAP_INIT_SIZE));
    if (ret < 0) {
        my_printf("Heap init failed\n");
        return -1;
    }
    heap_end = (void *)ret;
    
    heap_start->magic = MAGIC_HEADER;
    heap_start->size = HEAP_INIT_SIZE - sizeof(struct block);
    heap_start->free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;

    return 0;
}

void *my_malloc(size_t size)
{
    struct block *n = find_free(size);
    if (!n) 
    {
        struct block *extnd = extend_heap(size + HEAP_INIT_SIZE);
        if (!extnd) 
        {
            my_printf("malloc failed\n");
            return NULL;
        }
        n = find_free(size);
        if (!n) {
            my_printf("malloc failed after extension\n");
            return NULL;
        }
    }

    return (void *)((char *)n + sizeof(struct block));
}

void my_free(void *ptr)
{
    struct block *f = (struct block *)((uintptr_t)ptr - sizeof(struct block));

    if (f->magic == MAGIC_HEADER) {
        if (f->free) {
            my_printf("double free detected!\n");
            return;
        }
        f->free = 1;
        coalesce(f);
    } else {
        my_printf("invalid free detected!\n");
    }
}
