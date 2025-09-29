
#ifndef _MACHINE_H
#define _MACHINE_H

#if __SIZEOF_POINTER__ == 8
typedef long intptr_t;
typedef unsigned long uintptr_t;
typedef unsigned long size_t;
#define MACHINE_ALIGNMENT 8
#elif __SIZEOF_POINTER__ == 4
typedef int intptr_t;
typedef unsigned int uintptr_t;
typedef unsigned int size_t;
#define MACHINE_ALIGNMENT 4
#else
#error "Unsupported pointer size"
#endif

#define ISUNALIGNED(x)      ((((size_t)x) % MACHINE_ALIGNMENT) != 0U)
#define NULL (void *)0

typedef size_t word_t __attribute__((__may_alias__));
typedef unsigned char byte_t __attribute__((__may_alias__));

#endif