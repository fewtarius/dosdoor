/*
 * sigsegv.c - macOS signal fault handling for dosdoor
 * Minimal implementation for software CPU emulation
 */
#include "config.h"
#include "emu.h"
#include "cpu.h"

#include "doshelpers.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_exception_info(struct sigcontext_struct *scp)
{
    fprintf(stderr, "dosdoor: exception at eip=0x%lx\n", scp->eip);
}

int dosemu_fault1(int signal, struct sigcontext_struct *scp)
{
    /* In software CPU emulation mode, SIGSEGV means a real bug */
    fprintf(stderr, "dosdoor: unexpected signal %d\n", signal);
    print_exception_info(scp);
    return 0;
}

void dosemu_fault(int signal, siginfo_t *si, void *uc)
{
    struct sigcontext_struct sc;
    memset(&sc, 0, sizeof(sc));
    dosemu_fault1(signal, &sc);
    _exit(1);
}
