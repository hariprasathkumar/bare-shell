
#include "memory.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

void *my_memset(void *s, int c, size_t n)
{
    if (!n) return s;
    if (n < 16) 
    {
        byte_t *l = s;
        switch (n)
        {
            case 15: *l++ = (unsigned char)c; case 14: *l++ = (unsigned char)c; case 13: *l++ = (unsigned char)c; 
            case 12: *l++ = (unsigned char)c; case 11: *l++ = (unsigned char)c; case 10: *l++ = (unsigned char)c;
            case  9: *l++ = (unsigned char)c; case  8: *l++ = (unsigned char)c; case  7: *l++ = (unsigned char)c;
            case  6: *l++ = (unsigned char)c; case  5: *l++ = (unsigned char)c; case  4: *l++ = (unsigned char)c;
            case  3: *l++ = (unsigned char)c; case  2: *l++ = (unsigned char)c; case  1: *l++ = (unsigned char)c;
        }
    }
    else 
    {
        byte_t *l = s;
        size_t misaligned = (uintptr_t)s % MACHINE_ALIGNMENT;
        misaligned = min(misaligned, n);

        for (size_t i = 0 ; i < misaligned; l++, i++)
        {
            *l = (unsigned char)c;
        }
        size_t remaining = n - misaligned;

        if (remaining == 0) return s;

        size_t val = (unsigned char)c;
        val |= (val << 8);
        val |= (val << 16);

        if (MACHINE_ALIGNMENT == 8)
        {
            val |= (val << 32);
        }

        
        size_t rest = remaining / MACHINE_ALIGNMENT;
        word_t *w = (word_t *)l;
        
        for ( ; rest ; rest--, w++ ) *w = val;

        l = (byte_t *)w;
        size_t tail = remaining % MACHINE_ALIGNMENT;

        switch (tail)
        {
            #if MACHINE_ALIGNMENT == 8
            case 7:
                *l++ = (unsigned char) c;
            case 6:
                *l++ = (unsigned char) c;
            case 5:
                *l++ = (unsigned char) c;
            case 4:
                *l++ = (unsigned char) c;
            #endif
            case 3:
                *l++ = (unsigned char) c;
            case 2:
                *l++ = (unsigned char) c;
            case 1:
                *l++ = (unsigned char) c;
        }
    }
    return s;
}

void *my_memcpy(void *restrict dest, const void *restrict src, size_t n)
{

    size_t dest_misaligned =  (uintptr_t)dest % MACHINE_ALIGNMENT;
    size_t src_misaligned  =  (uintptr_t)src  % MACHINE_ALIGNMENT;
    byte_t *d = dest;
    const byte_t *s = src;
    
    if (dest_misaligned == src_misaligned)
    {
        size_t misaligned = min(dest_misaligned, n);
        size_t remaining = n - misaligned;

        switch (misaligned)
        {
            #if MACHINE_ALIGNMENT == 8
            case 7:
                *d++ = *s++;
            case 6:
                *d++ = *s++;
            case 5:
                *d++ = *s++;
            case 4:
                *d++ = *s++;
            #endif
            case 3:
                *d++ = *s++;
            case 2:
                *d++ = *s++;
            case 1:
                *d++ = *s++;
        }

        word_t *dst = (word_t *)d;
        const word_t *srx = (const word_t *)s;

        size_t rest = remaining / MACHINE_ALIGNMENT;
        for ( ; rest; rest--, *dst++ = *srx++);

        size_t tail = remaining % MACHINE_ALIGNMENT;

        d = (byte_t *)dst, s = (byte_t *)srx;

        switch (tail)
        {
            #if MACHINE_ALIGNMENT == 8
            case 7:
                *d++ = *s++;
            case 6:
                *d++ = *s++;
            case 5:
                *d++ = *s++;
            case 4:
                *d++ = *s++;
            #endif
            case 3:
                *d++ = *s++;
            case 2:
                *d++ = *s++;
            case 1:
                *d++ = *s++;
        }

    }
    else
    {
        while (n) {
            *d++ = *s++, n--;
        }
    }

    return dest;
}

void *my_memmove(void *dest, const void *src, size_t n)
{
    if ((uintptr_t)dest == (uintptr_t)src) return dest;

    if ((dest + n <= src) || (src + n <= dest)) {
        return my_memcpy(dest, src, n);
    }

    byte_t *d = (byte_t *)dest;
    const byte_t *s = (const byte_t *)src;

    if ((uintptr_t)d > (uintptr_t)s) {
        d += n, s += n;
        if ( n < 16 )
        {
            switch (n) {
                case 15: *--d = *--s;
                case 14: *--d = *--s;
                case 13: *--d = *--s;
                case 12: *--d = *--s;
                case 11: *--d = *--s;
                case 10: *--d = *--s;
                case  9: *--d = *--s;
                case  8: *--d = *--s;
                case  7: *--d = *--s;
                case  6: *--d = *--s;
                case  5: *--d = *--s;
                case  4: *--d = *--s;
                case  3: *--d = *--s;
                case  2: *--d = *--s;
                case  1: *--d = *--s;
            }
        }
        else
        {
            if ( ((uintptr_t)d % MACHINE_ALIGNMENT) == ((uintptr_t)s % MACHINE_ALIGNMENT))
            {
                for ( ; ISUNALIGNED(d) && n ; ) n--, *--d = *--s;
                
                while ( n >= MACHINE_ALIGNMENT) {
                    n -= MACHINE_ALIGNMENT, d -= MACHINE_ALIGNMENT, s -= MACHINE_ALIGNMENT;
                    *(word_t *)d = *(const word_t *)s;
                }

            }
            // backward
            while (n) n--, *--d = *--s;
        }
    } else {
        if ( n < 16 )
        {
            switch (n) {
                case 15: *d++ = *s++;
                case 14: *d++ = *s++;
                case 13: *d++ = *s++;
                case 12: *d++ = *s++;
                case 11: *d++ = *s++;
                case 10: *d++ = *s++;
                case  9: *d++ = *s++;
                case  8: *d++ = *s++;
                case  7: *d++ = *s++;
                case  6: *d++ = *s++;
                case  5: *d++ = *s++;
                case  4: *d++ = *s++;
                case  3: *d++ = *s++;
                case  2: *d++ = *s++;
                case  1: *d++ = *s++;
            }
        }
        else
        {
            if ( ((uintptr_t)d % MACHINE_ALIGNMENT) == ((uintptr_t)s % MACHINE_ALIGNMENT))
            {
                for ( ; ISUNALIGNED(d) && n ; ) n--, *d++ = *s++;
                
                while ( n >= MACHINE_ALIGNMENT ) { 
                    n -= MACHINE_ALIGNMENT; *(word_t *)d = *(const word_t *)s;
                    d += MACHINE_ALIGNMENT, s += MACHINE_ALIGNMENT;
                }

            }
            // forward
            while (n) n--, *d++ = *s++; 
        }
    }

    return dest;
}

int my_memcmp(const void *s1, const void *s2, size_t n)
{
    if (!n) return 0;

    const byte_t *b1 = (const byte_t *)s1;
    const byte_t *b2 = (const byte_t *)s2;

    if ( ((uintptr_t)b1 % MACHINE_ALIGNMENT) == ((uintptr_t)b2 % MACHINE_ALIGNMENT) )
    {
        while (ISUNALIGNED(b1) && n) {
            if (*b1 != *b2)  return (int)(*b1 - *b2);
            n--, b1++, b2++;
        }
        while ( n >= MACHINE_ALIGNMENT ) {
            if ( *(word_t *)b1 != *(word_t *)b2) {
                size_t diff = (*(word_t *)b1) ^ (*(word_t *)b2);
                #if MACHINE_ALIGNMENT == 8
                size_t index = __builtin_ctzll((unsigned long long)diff) / 8;
                #else 
                size_t index = __builtin_ctz(diff) / 8;
                #endif

                return (int)(*(b1 + index) - *(b2 + index));
            }
            b1 += MACHINE_ALIGNMENT, b2 += MACHINE_ALIGNMENT;
            n -= MACHINE_ALIGNMENT;
        }
    }
    while (n) {
        if (*b1 != *b2) return (int)(*b1 - *b2);
        b1++, b2++, n--;
    }

    return 0;
}