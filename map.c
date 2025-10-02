
#include "heap.h"
#include "string.h"
#include "print.h"
#include "map.h"
#include "memory.h"

#define LOAD_FACTOR_THRESHOLD    (0.7f)

static inline size_t FNV1a(const char *cmd) 
{
/*
    https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    algorithm fnv-1a is
    hash := FNV_offset_basis

    for each byte_of_data to be hashed do
        hash := hash XOR byte_of_data
        hash := hash × FNV_prime

    return hash 
*/
    const size_t FNV_prime = 0x00000100000001b3;
    size_t output = 0xcbf29ce484222325;

    while (*cmd) {
        output ^= (unsigned long)(*cmd++);
        output *= FNV_prime;
    }

    return output;
}

static size_t hash1(struct hash *map, const char *cmd) 
{
    return FNV1a(cmd) % map->table_size;
}

static size_t hash2(struct hash *map, const char *cmd)
{
    return 1 + (FNV1a(cmd) % (map->table_size - 1));
}

static inline void update_load_factor(struct hash *map)
{
    if (map)
    map->load_factor = (float)map->num_of_elements / map->table_size;
}

static char isPrime(size_t i)
{
    if (i <= 1) return 0;
    if (i == 2) return 1;
    if (i % 2 == 0) return 0;

    for (size_t j = 3; j * j <= i; j++)
    {
        if (i % j == 0) return 0;
    }

    return 1; // no number below its sqrt is divisible
}

static long long getPrime(size_t start)
{
    while (!isPrime(start)) start++;
    return start;
}

int map_init(struct hash **hash, size_t no_of_slots) 
{    
    size_t taken_table_size = getPrime(no_of_slots);

    *hash = (struct hash *) my_malloc(sizeof(struct hash));
    if (!*hash)
    {
        return -1;
    }

    my_memset(*hash, 0, sizeof(struct hash));

    (*hash)->hashmap = (struct key_value_pair **) my_malloc (taken_table_size * sizeof(struct key_value_pair *));
    if (!(*hash)->hashmap) 
    {
        return -1;
    }

    my_memset((*hash)->hashmap, 0, taken_table_size * sizeof(struct key_value_pair *));

    (*hash)->num_of_elements = 0;
    (*hash)->table_size = taken_table_size;
    (*hash)->load_factor = 0.0f;

    return 0;
}

/*
    Double hashing uses a hash for start index another has for step increment
    important thigs is the step is a coprime with table size 
    this is done by taking the table size - 1 modulo
    coprime ensures that all slots are vistied on a linear probe

    index = (hash1(key) + i*hash2(key)) %  table_size
    hash1(key) = key % table_size
    hash2(key) = 1 + (hash1(key) % N-1) => modulo N-1 ensure 0..N-2 range +1 is to avoid infinite loop / mov ethe step so 1..N-1 (no 0, causes loop) 
*/

static struct hash *rehash(struct hash *map)
{
    long long cur_size = map->table_size;
    long long new_size = getPrime(cur_size * 2);

    struct hash *newmap = (struct hash *) my_malloc(sizeof(struct hash));
    if (!newmap) 
    {
        return NULL;
    }

    newmap->hashmap = (struct key_value_pair **) my_malloc (new_size * sizeof(struct key_value_pair *));
    if (!newmap->hashmap) 
    {
        free_wrapper(newmap, "rehash");
        return NULL;
    }
    newmap->table_size = new_size;
    newmap->hash_fn = map->hash_fn;
    newmap->num_of_elements = 0;

    for (size_t i = 0; i < cur_size; i++) 
    {
        struct key_value_pair *entry = map->hashmap[i];
        if (entry && entry != TOMBSTONE) 
        {
            const char *cmd = (const char *)entry->cmd;
            // probe into new table until empty slot
            for (size_t j = 0; j < new_size; j++) 
            {
                size_t index = (hash1(newmap, cmd) + j * hash2(newmap, cmd)) % new_size;
                if (!newmap->hashmap[index]) 
                {
                    newmap->hashmap[index] = entry; // move pointer directly
                    newmap->num_of_elements++;
                    break;
                }
            }
        }
    }

    // free only the old table array, not the key_value_pair structs
    free_wrapper(map->hashmap, "rehashed");
    free_wrapper(map, "rehashed");

    return newmap;
}

struct sym_entry *map_lookup(struct hash **hash, const char *cmd)
{
    if (!hash || !*hash) 
    {
        return NULL;
    }

    struct hash *map = *hash;
    for (size_t i = 0; i < map->table_size; i++) 
    {
        size_t index = (hash1(map, cmd) + i * hash2(map, cmd)) % map->table_size;

        if (!map->hashmap[index]) return NULL;
        else if (map->hashmap[index] == TOMBSTONE) continue;
        else if (my_strncmp(cmd, map->hashmap[index]->cmd, 512) == 0)
        {
            return &map->hashmap[index]->val;
        }
        else
        {
            // continue probe
        }
    }

    return NULL;
}

void free_wrapper(void * p, const char *owner)
{
    //my_printf("free by %s ptr %d\n", owner, p);
    my_free(p);
}

void free_map(struct hash *map)
{
    for (size_t i = 0; i < map->table_size; i++) 
    {
        if (map->hashmap[i] && map->hashmap[i] != TOMBSTONE) 
        {
            free_wrapper(map->hashmap[i], "free_map: i");
        }
    }
    if (map)
    {
        free_wrapper(map->hashmap, "free_map: table");
        free_wrapper(map, "free_map: p");
    }
}

void map_insert(struct hash **hash, const char *cmd, const char *path, int builtin)
{
    if (!hash || !*hash) 
    {
        return;
    }

    struct hash *map = *hash;
    for (size_t i = 0; i < map->table_size; i++) 
    {
        size_t index = (hash1(map, cmd) + i * hash2(map, cmd)) % map->table_size;

        if (!map->hashmap[index] || map->hashmap[index] == TOMBSTONE)
        {
            struct key_value_pair *n = (struct key_value_pair *) my_malloc (sizeof(struct key_value_pair));
            (void)my_strncpy(n->cmd, cmd, 512);
            (void)my_strncpy(n->val.path, path, 512);
            n->val.builtin = builtin;

            map->hashmap[index] = n;
            map->num_of_elements++;
            
            update_load_factor(map);
            if (map->load_factor > LOAD_FACTOR_THRESHOLD)
            {
                struct hash *new_hash = rehash(map);
                if (new_hash)
                {
                    *hash = new_hash;
                }
            }
            break;
        }
        else if (map->hashmap[index] && my_strncmp(cmd, map->hashmap[index]->cmd, 512) == 0) // overwrite
        {
            my_strncpy(map->hashmap[index]->val.path, path, 512);
            map->hashmap[index]->val.builtin = builtin;
            break;
        }
    }
}

void map_delete(struct hash **hash, const char *cmd)
{
    if (!hash || !*hash) 
    {
        return;
    }

    struct hash *map = *hash;
    for (size_t i = 0; i < map->table_size; i++) 
    {
        size_t index = (hash1(map, cmd) + i * hash2(map, cmd)) % map->table_size;

        if (!map->hashmap[index]) return; // key not found
        if (map->hashmap[index] == TOMBSTONE) continue;
        if (my_strncmp(map->hashmap[index]->cmd, cmd, 512) == 0) 
        {
            free_wrapper(map->hashmap[index], "Map delete");
            map->hashmap[index] = TOMBSTONE;
            map->num_of_elements--;
            update_load_factor(map);
            return;
        }
    }
}

void map_print_all(struct hash *hash)
{
    for (long long i = 0; i < hash->table_size; i++)
    {
        if (hash->hashmap[i] && hash->hashmap[i] != TOMBSTONE)
        {
            my_printf("Map contents: key=%s val=%s builtin=%d\n", hash->hashmap[i]->cmd, hash->hashmap[i]->val.path, hash->hashmap[i]->val.builtin);
        }
    }
}