
#ifndef _HEAP_H
#define _HEAP_H

#include "machine.h"

#define HEAP_INIT_SIZE      8*1024 // 8kB
#define MAGIC_HEADER        0x55AA

long heap_init() ;
void *my_malloc(size_t size);
void my_free(void *ptr);

void walk(void);;

#endif