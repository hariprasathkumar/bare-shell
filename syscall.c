
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