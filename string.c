
#include "string.h"
#include "machine.h"


// if 0 -> 0-1 => 0xff & 0x80 => 0x80 & 0x80 => != 0
#define ONES            ((~(size_t)0) / 0xFFU) // 0x0101..
#define HIGHS           (ONES << 7U) // 0x8080..
#define NONZEROES(x)    (((x - ONES) & HIGHS & ~x) == 0U)

size_t my_strlen(const char *s)
{
    const char *a = s;

    // move pointer to end
    for ( ; ISUNALIGNED(a); a++ ) if (!*a) return a - s;
    const size_t *w = (const size_t *)a;
    for ( ; NONZEROES(*w) ; w++);
    for (a = (const char *)w; *a; a++ );

    return a-s;
}

int my_strcmp(const char *s1, const char *s2)
{
    while ( *s1 == *s2 && *s1 ) { s1++, s2++; }
    return *s1-*s2;
}

// extended ascii set
int my_strncmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *str1 = s1, *str2 = s2;
    if (!n) return 0;
    for ( n-- ; *str1 == *str2 && *str1 && n; str1++, str2++, n-- );
    return *str1-*str2;
}

char *my_strncpy(char *restrict dest, const char *restrict src, size_t n)
{
    char *dest_s = dest;
    const char *src_s = src;
    
    if (!n) return dest;

    while (*src_s && n) {
        *dest_s++ = *src_s++, n--;
    }

    while (n) {
        *dest_s = '\0', dest_s++, n--;
    }

    return dest;
}