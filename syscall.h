
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "machine.h"

typedef unsigned short		umode_t;

struct linux_dirent64 {
	unsigned long		d_ino;
	signed long 		d_off;
	unsigned short	    d_reclen;
	unsigned char	    d_type;
	char		        d_name[];
};

/* Source : v6.14, linux/arch/x86/entry/syscalls/syscall_64.tbl */
long sys_read(unsigned int fd, char *buf, size_t count);
long sys_write(unsigned int fd, const char *buf, size_t count);
long sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
long sys_brk(unsigned long brk);
long sys_openat(int dfd, const char *filename, int flags, umode_t mode);
long sys_getdents64(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count);
long sys_close(unsigned int fd);
long sys_access(const char *filename, int mode);
#endif
