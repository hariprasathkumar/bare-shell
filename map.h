
#ifndef _MAP_H
#define _MAP_H

#include "machine.h"

#define TOMBSTONE                ((void *)-1)
struct hash 
{
    struct key_value_pair **hashmap;
    long long num_of_elements;
    long long table_size;
    float load_factor;
    size_t (*hash_fn)(struct hash *hash, const char *cmd);
};

struct sym_entry {
    char path[512];
    int builtin;
};

struct key_value_pair 
{
    char cmd[512];
    struct sym_entry val;
};

int map_init(struct hash **hash, size_t no_of_slots);
struct sym_entry *map_lookup(struct hash **hash, const char *cmd);
void map_insert(struct hash **hash, const char *cmd, const char *path, int builtin);
void map_delete(struct hash **hash, const char *cmd);
void free_map(struct hash *map);
void free_wrapper(void * p, const char *owner);
void map_print_all(struct hash *hash);

#endif