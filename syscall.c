
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