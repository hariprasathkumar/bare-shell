
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "machine.h"

/* Source : v6.14, linux/arch/x86/entry/syscalls/syscall_64.tbl */
long sys_read(unsigned int fd, char *buf, size_t count);
long sys_write(unsigned int fd, const char *buf, size_t count);
long sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
long sys_brk(unsigned long brk);

#endif
