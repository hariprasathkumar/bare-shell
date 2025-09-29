
#include "stdlib.h"
#include "ctype.h"
#include "machine.h"

static int isValid(int base, char val)
{
    if (val >= '0' && val <= '9') return val - '0';
    else if (val >= 'a' && val <= 'z') return val - 'a' + 10;
    else if (val >= 'A' && val <= 'Z') return val - 'A' + 10;

    return -1;
}

static long int my_strtol_generic(const char *nptr, char **restrict endptr, int base, long int max, long int min)
{
    const char *c = nptr;
    long int sign = 1;
    long int val = 0;
    char found = 0;

    if (endptr)
        *endptr = (char *)nptr;

    while (isspace(*c)) {
        c++;
    }

    if (*c == '-') {
        c++;
        sign = -1;
    } else if (*c == '+') {
        c++;
    }

    if (base < 0 || base == 1 || base > 36) {
        return 0;
    }

    if (base == 0) {
        if (*c == '0') {
            c += 1;
            if (endptr) *endptr = (char *)c;
            if (c[0] == 'x' || c[0] == 'X') {
                base = 16;
                c += 1;  
            } else {
                base = 8;  
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (c[0] == '0' && (c[1] == 'x' || c[1] == 'X'))
            c += 2;
    }

    int digit;

    long int cutoff = max / base;
    long int cutlim = max % base;
    char overflow = 0;
    while ( (digit = isValid(base, *c)) != -1)
    {
        if (digit >= base) break;
        if (val > cutoff || (val == cutoff && digit > cutlim)) {
            overflow = 1;
        } else {
            val = (val * base) + digit;
        }
        
        c++;
        if (endptr) *endptr = (char *)c;
    }

    if (overflow)
    {
        if (sign == -1) val = min;
        else val = max;
    }
    else 
    {
        val *= sign;
    }

    return val;
}

long int my_strtol(const char *restrict nptr, char **restrict endptr, int base)
{
    return my_strtol_generic(nptr, endptr, base, __LONG_MAX__, -__LONG_MAX__ - 1);
}

int my_atoi(const char *nptr)
{
    return my_strtol_generic(nptr, NULL, 10, __INT_MAX__, -__INT_MAX__ - 1);
}

char *my_itoa(int value, char *str, int base)
{

}
