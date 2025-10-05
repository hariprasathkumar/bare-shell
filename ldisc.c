
#include "ldisc.h"
#include "syscall.h"
#include "print.h"
#include "string.h"
#include "memory.h"
#include "history.h"

/* --------------- KERNEL STRUCTURES --------------- */
// int tty_mode_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg)
#define NCCS        19

#define TCGETS		0x5401
#define TCSETS		0x5402
#define ISIG	    0x00001 // send control characters 
#define ICANON	    0x00002 // to disable line buffering 
#define ECHO	    0x00008 // to disable echo 
#define IEXTEN	    0x08000 // disable signals

#define ICRNL	0x100			/* Map CR to NL on input */
#define IXON	0x0400
#define IXOFF	0x1000

#define OPOST	0x01			/* Perform output processing */

#define VTIME		 5
#define VMIN		 6

// termios2

typedef unsigned int	tcflag_t;
typedef unsigned int	speed_t;
typedef unsigned char	cc_t;

struct termios {
	tcflag_t c_iflag;		/* input mode flags */
	tcflag_t c_oflag;		/* output mode flags */
	tcflag_t c_cflag;		/* control mode flags */
	tcflag_t c_lflag;		/* local mode flags */
	cc_t c_line;			/* line discipline */
	cc_t c_cc[NCCS];		/* control characters */
};

struct termios flags;
struct termios default_flags;
/* --------------- KERNEL STRUCTURES --------------- */

#define LDISC_READ_EOF     0
#define LDISC_READ_ERR    -1

#define LDISC_MAX_LINE     512

char ldisc_line[LDISC_MAX_LINE];

void ldisc_init(void) {
    long ret;
    const char *err  = "Cant get attributes!\n";
    const char *err2 = "Cant set attributes!\n";

    ret = sys_ioctl(0, TCGETS, (unsigned long)&flags);
    if (ret < 0) sys_write(1, err, my_strlen(err));

    my_memcpy(&default_flags, &flags, sizeof(struct termios));

    /* Refer /include/uapi/asm-generic/termbits.h */
    flags.c_lflag &= ~( ISIG | ECHO | IEXTEN | ICANON);
    flags.c_iflag &= ~( ICRNL | IXON | IXOFF );
    flags.c_oflag &= ~OPOST;

    flags.c_cc[VMIN]  = 1;
    flags.c_cc[VTIME] = 0;

    ret = sys_ioctl(0, TCSETS, (unsigned long)&flags);
    if (ret < 0) sys_write(1, err2, my_strlen(err2));

}

void ldisc_deinit(void) {
    const char *err = "Cant set attributes!\n";

    long ret = sys_ioctl(0, TCSETS, (unsigned long)&default_flags);
    if (ret < 0) sys_write(1, err, my_strlen(err));  
}

extern void smart_prompt(int laststatus);

void redraw(int last_status, size_t *cursor)
{
    my_printf("\r");
    smart_prompt(last_status);
    my_printf("\x1b[0K");  // erase till end of line
    my_printf("%s", ldisc_line);
    size_t len = my_strlen(ldisc_line);
    for (size_t j = *cursor; j < len; j++) {
        my_printf("\x1b[D");
    }
}

const char *my_readline(int last_status)
{
    char c;
    size_t len = 0;
    size_t cursor_pos = 0;
    long ret;
    long ctrlchar = 0;

    my_memset(ldisc_line, 0, LDISC_MAX_LINE);
    smart_prompt(last_status);
    while ((ret = sys_read(0, &c, 1)) == 1 && len < LDISC_MAX_LINE - 1) {
        if (ctrlchar == 0 && c >= 32 && c <= 126) {
            if (cursor_pos != len) my_memmove(ldisc_line + cursor_pos + 1, ldisc_line + cursor_pos, len - cursor_pos + 1);
            ldisc_line[cursor_pos++] = c;
            len++;
            redraw(last_status, &cursor_pos);
        } else if (c == '\n' || c == '\r') {
            ldisc_line[len] = '\0'; 
            break;
        } else if (c == 0x7F || c == 8) { // back spcce or delete
            // send move cursor left \b, write space move cursor 
            if (cursor_pos > 0) {
                my_memmove(ldisc_line + cursor_pos - 1, ldisc_line + cursor_pos, len - cursor_pos + 1);
                cursor_pos--;
                len--;
                redraw(last_status, &cursor_pos);
                // Move cursor back where it belongs
                for (size_t k = cursor_pos; k < len; k++) {
                    my_printf("\x1b[D");
                }
            }
        } else if (c == 0x03) { // ctrl + c
            len = 0;
            my_printf("^C\r\n");
            break;
        } else if (c == 0x1b) { // arrow keys
            ctrlchar = 1;
        } else if (ctrlchar == 1 && c == '[') {
            ctrlchar = 2;
        } else if (ctrlchar == 2 && c == 'D') {
            // left arrow
            if (cursor_pos) {
                my_printf("%s", "\x1b[D");
                ctrlchar = 0;
                cursor_pos--;
            }
        } else if (ctrlchar == 2 && c == 'C') {
            // right arrow
            if (cursor_pos < len) {
                my_printf("%s", "\x1b[C");
                ctrlchar = 0;
                cursor_pos++;
            }
        } else if (ctrlchar == 2 && c == 'A') {
            // up arrow
            ctrlchar = 0;
            const char *prev = get_previous_history();
            if (prev)
            {
                my_strncpy(ldisc_line, prev, LDISC_MAX_LINE);
                cursor_pos = len = my_strlen(ldisc_line);
                redraw(last_status, &cursor_pos);
            }
        } else if (ctrlchar == 2 && c == 'B') {
            // down arrow
            ctrlchar = 0;
            const char *next = get_next_history();
            if (next)
            {
                my_strncpy(ldisc_line, next, LDISC_MAX_LINE);
                cursor_pos = len = my_strlen(ldisc_line);
                redraw(last_status, &cursor_pos);
            }
        } else {
            ctrlchar = 0;
        }
    }

    my_printf("%s", "\r\n");

    if (len == LDISC_MAX_LINE - 1) ldisc_line[len] = '\0';

    return len == 0 ? NULL : ldisc_line;
}