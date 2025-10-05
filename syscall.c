
#include "syscall.h"

/* Arch/ABI    Instruction           System  Ret  Ret  Error    Notes
                                     call #  val  val2                  */
/* x86-64      syscall               rax     rax  rdx  -        5       */

/* Arch/ABI      arg1  arg2  arg3  arg4  arg5  arg6  arg7  Notes */
/* x86-64        rdi   rsi   rdx   r10   r8    r9    -           */

// 0	common	read			sys_read
long sys_read(unsigned int fd, char *buf, size_t count)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(0), "D"((long)fd), "S"((long)buf), "d"((long )count) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

// 1	common	write			sys_write
long sys_write(unsigned int fd, const char *buf, size_t count)
{
    long retVal1 = 0; // linux has only one ret

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1) /* outputs */
        : "a"(1), "D"((long)fd), "S"((long)buf), "d"((long)count)/* inputs */
        : "rcx", "r11", "memory"  // clobbered, syscal instruction overwrites ret address in rcx, eflags in r11, safe to so memory
    );

    return retVal1;
}

// 16	64	ioctl			sys_ioctl
long sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(16), "D"((long)fd), "S"((long)cmd), "d"((long )arg) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//12	common	brk			sys_brk
long sys_brk(unsigned long brk)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(12), "D"((long)brk) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//257	common	openat			sys_openat
long sys_openat(int dfd, const char *filename, int flags, umode_t mode)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(257), "D"((long)dfd), "S"((long)filename), "d"((long )flags), "r"((long)mode) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//217	common	getdents64		sys_getdents64
long sys_getdents64(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(217), "D"((long)fd), "S"((long)dirent), "d"((long )count) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//3	common	close			sys_close
long sys_close(unsigned int fd)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(3), "D"((long)fd) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//21	common	access			sys_access
long sys_access(const char *filename, int mode)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(21), "D"((long)filename), "S"((long)mode) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//57	common	fork			sys_fork
long sys_fork(void)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(57) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//59	64	execve			sys_execve
long sys_execve(const char *filename, const char *const *argv, const char *const *envp)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(59), "D"((long)filename), "S"((long)argv), "d"((long)envp) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//33	common	dup2			sys_dup2
long sys_dup2(unsigned int oldfd, unsigned int newfd)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(33), "D"((long)oldfd), "S"((long)newfd) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//61	common	wait4			sys_wait4
long sys_wait4(pid_t pid, int *stat_addr, int options, void *ru)
{
    long retVal1;
    register long r10 asm("r10") = (long)ru;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(61), "D"((long)pid), "S"((long)stat_addr), "d"((long)options), "r"((long)r10) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//293	common	pipe2			sys_pipe2
long sys_pipe2(int *fildes, int flags)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(293), "D"((long)fildes), "S"((long)flags) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

//80	common	chdir			sys_chdir
long sys_chdir(const char *filename)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(80), "D"((long)filename) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}


//60	common	exit			sys_exit	
long sys_exit(int error_code)
{
    long retVal1 = 0;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(60), "D"((long)error_code) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1;
}

// flow:
// user code runs
// kernel signals
// kernel pushes a dummy stack frame of user process
// set return address in stack to sa_restorer
// signal runs
// signal returns
// execute sa_restorer
// context restored by kernel and remove the stack frame
// execution resumes
__attribute__((naked)) void sys_rt_sigreturn(void)
{
    asm volatile (
        "mov $15, %rax\n\t"
        "syscall\n\t"
    );
}

//13	64	rt_sigaction		sys_rt_sigaction
long sys_rt_sigaction(int signum,
                      const struct sigaction *newact,
                      struct sigaction *oldact,
                      size_t sigsetsize)
{
    long ret;
    register long r10 asm("r10") = (long)sigsetsize;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(13),
          "D"(signum),
          "S"(newact),
          "d"(oldact),
          "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

//79	common	getcwd			sys_getcwd
long sys_getcwd(char *buf, unsigned long size)
{
    long retVal1;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(79), "D"((long)buf), "S"((long)size) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1; 
}

//63	common	uname			sys_newuname
long sys_newuname(struct new_utsname *name)
{
    long retVal1;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(79), "D"((long)name) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1; 
}

//102	common	getuid			sys_getuid
long sys_getuid(void)
{
    long retVal1;

    asm volatile(
        "syscall\n\t"
        : "=a"(retVal1)
        : "a"(102) /* inputs */
        : "rcx", "r11", "memory"
    );

    return retVal1; 
}
