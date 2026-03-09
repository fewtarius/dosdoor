/*
 * macOS compatibility header for dosemu signal context
 * Provides struct sigcontext with x86 register fields
 * used by DPMI and signal handling code
 */
#ifndef _DOSDOOR_SIGCOMPAT_H
#define _DOSDOOR_SIGCOMPAT_H

#ifdef __APPLE__

#include <signal.h>

/* x86 FPU state - used by DPMI client */
struct _fpstate {
    unsigned short cwd;
    unsigned short swd;
    unsigned short twd;
    unsigned short fop;
    unsigned long rip;
    unsigned long rdp;
    unsigned int mxcsr;
    unsigned int mxcsr_mask;
    unsigned int st_space[32];
    unsigned int xmm_space[64];
    unsigned int padding[24];
};

/* x86 signal context - register state for dosemu signal handling */
struct sigcontext {
    unsigned long gs;
    unsigned long fs;
    unsigned long es;
    unsigned long ds;
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
    unsigned long trapno;
    unsigned long err;
    unsigned long eip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long esp_at_signal;
    unsigned long ss;
    struct _fpstate *fpstate;
    unsigned long oldmask;
    unsigned long cr2;
};

#endif /* __APPLE__ */
#endif /* _DOSDOOR_SIGCOMPAT_H */
