
#include "ctype.h"

int isdigit(int c)
{
    return (unsigned char)c - '0' <= '9' - '0';
}

int isspace(int c)
{
    switch ((unsigned char)c)
    {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case '\f':
        case '\v':
            return 1;
    }

    return 0;
}