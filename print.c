
#include "print.h"
#include "string.h"
#include "syscall.h"

static void itostr(char *str, size_t size, size_t *i, unsigned long int val, int *len)
{
    if ( val < 10) {
        str[(*i)++] = val + '0';
        (*len)++;
        return;
    }

    int digit = val % 10;
    int rest = val / 10;
    
    itostr(str, size, i, rest, len);
    if (size != 0 && *i < size - 1)  str[(*i)++] = digit + '0';
    (*len)++;
}

int my_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    size_t i = 0;
    int total_length = 0;

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if ( *format != '\0')
            {
                switch (*format) {
                    case 'd': // signed int
                    case 'i': 
                    {
                        int v = va_arg(ap, int);
                        if (v < 0) 
                        {
                            if (size != 0 && i < size - 1) str[i++] = '-';
                            total_length++;
                            unsigned int uv = (v < 0) ? (unsigned int)(-(long long)v) : (unsigned int)v;
                            itostr(str, size, &i, uv, &total_length);
                        }
                        else
                        {
                            itostr(str, size, &i, v, &total_length);
                        }
                        break;
                    }
                    case 'u':
                    {
                        unsigned int v = va_arg(ap, unsigned int);
                        itostr(str, size, &i, v, &total_length);
                        break;
                    }

                    case 's': // string
                    {
                        const char *s =  va_arg(ap, char *);
                        size_t si = 0;
                        const char *nul = "(null)";
                        if (!s) s = nul;

                        while (s[si] != '\0') {
                            if (size != 0 && i < size - 1 ) str[i] = s[si];
                            i++, si++;
                        }
                        total_length += si;
                        break;
                    }

                    case 'c': // chars
                    {
                        int c = va_arg(ap, int);
                        if (size != 0 && i < size - 1) str[i] = (unsigned char)c; // defautl promotion
                        printf("%d\n", str[i]);
                        i++, total_length++;
                        break;
                    }

                    case '%': // literal
                    {
                        if (size != 0 && i < size - 1)  str[i] = '%';
                        i++, total_length++;
                        break;
                    }
                }
                format++;
            }
        } else {
            if (size != 0 && i < size - 1) str[i] = *format;
            i++, total_length++, format++;
        }
    }
    
    if (size != 0) str[(i < size) ? i : size - 1] = '\0';

    return total_length;
}

int my_printf(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    
    char buf[1024];

    int written = my_vsnprintf(buf, 1024, format, ap);

    sys_write(1, buf, written);

    va_end(ap);

    return written;
}