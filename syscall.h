
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

typedef int pid_t;

#define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#define WIFEXITED(status)   (((status) & 0x7f) == 0)
#define WTERMSIG(status)    ((status) & 0x7f)
#define WIFSIGNALED(status) ((status) - 1 < 0xff)

#define WNOHANG		0x00000001
#define WUNTRACED	0x00000002
#define WSTOPPED	WUNTRACED
#define WEXITED		0x00000004
#define WCONTINUED	0x00000008
#define WNOWAIT		0x01000000	/* Don't reap, just poll status.  */

#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002

#define O_CREAT		00000100	/* not fcntl */
#define O_TRUNC		00001000	/* not fcntl */
#define O_APPEND	00002000
#define O_DIRECTORY	00200000	

#define AT_FDCWD		-100 

#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

// uapi/asm-generic/signal.h
#define SIGCHLD		17
// uapi/asm-generic/signal-defs.h
#define SA_RESTART		0x10000000 // restart syscall if interrupted by signal
#define SA_NOCLDSTOP	0x00000001 // flag to turn off SIGCHLD when children stop
#define SA_RESTORER		0x04000000 // Call to trampoline

typedef unsigned long sigset_t[16]; // 128 bytes

struct sigaction {
    void (*sa_handler)(int);
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    sigset_t sa_mask;
};

#define __NEW_UTS_LEN 64

struct new_utsname {
	char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
};

/* Source : v6.14, linux/arch/x86/entry/syscalls/syscall_64.tbl 
				   /include/linux/syscalls.h
*/
long sys_read(unsigned int fd, char *buf, size_t count);
long sys_write(unsigned int fd, const char *buf, size_t count);
long sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
long sys_brk(unsigned long brk);
long sys_openat(int dfd, const char *filename, int flags, umode_t mode);
long sys_getdents64(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count);
long sys_close(unsigned int fd);
long sys_access(const char *filename, int mode);
long sys_fork(void);
long sys_execve(const char *filename, const char *const *argv, const char *const *envp);
long sys_dup2(unsigned int oldfd, unsigned int newfd);
long sys_wait4(pid_t pid, int *stat_addr, int options, /*struct rusage *ru */ void *ru);
long sys_pipe2(int *fildes, int flags);
long sys_chdir(const char *filename);
long sys_exit(int error_code);
__attribute__((naked)) void sys_rt_sigreturn(void);
long sys_rt_sigaction(int signal_number, const struct sigaction *install_new, struct sigaction *old_handler, size_t sigsetsize);
long sys_getcwd(char *buf, unsigned long size);
long sys_newuname(struct new_utsname *name);
long sys_getuid(void);
#endif
