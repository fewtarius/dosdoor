/*
 * signal.c - macOS signal handling for dosdoor
 * Port of Linux signal handling with SIGNAL_save queue mechanism
 * and SIGALRM_call housekeeping for timer, keyboard, serial, etc.
 */
#include "config.h"
#include "emu.h"
#include "cpu.h"
#include "timers.h"
#include "pic.h"
#include "memory.h"
#include "int.h"
#include "dpmi.h"
#include "serial.h"
#include "bios.h"
#include "video.h"
#include "iodev.h"
#include "keyboard.h"
#include "keyb_clients.h"
#include "cpu-emu.h"
#include "hma.h"
#include <sys/ucontext.h>
extern int in_vm86_emu;
extern unsigned long trans_addr, return_addr;

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

/* Signal queue mechanism - mirrors Linux implementation */
#define MAX_SIG_QUEUE_SIZE 50
static u_short SIGNAL_head = 0;
u_short SIGNAL_tail = 0;

struct SIGNAL_queue {
    void (*signal_handler)(void);
};
static struct SIGNAL_queue signal_queue[MAX_SIG_QUEUE_SIZE];

static void (*sig_handlers[32])(struct sigcontext_struct *);

/* Forward declarations */
static void SIGALRM_call(void);

/*
 * SIGNAL_save - queue a signal handler for later execution
 * Called from signal context, executes handler in main thread
 */
void SIGNAL_save(void (*signal_call)(void))
{
    signal_queue[SIGNAL_tail].signal_handler = signal_call;
    SIGNAL_tail = (SIGNAL_tail + 1) % MAX_SIG_QUEUE_SIZE;
    signal_pending = 1;
    if (in_dpmi)
        dpmi_return_request();
}

void addset_signals_that_queue(sigset_t *x)
{
    sigaddset(x, SIGALRM);
    sigaddset(x, SIGPROF);
    sigaddset(x, SIGIO);
    sigaddset(x, SIGCHLD);
}

void registersig(int sig, void (*handler)(struct sigcontext_struct *))
{
    if (sig >= 0 && sig < 32)
        sig_handlers[sig] = handler;
}

void init_handler(struct sigcontext_struct *scp)
{
    /* Block signals during handler execution */
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);
}

/*
 * SIGALRM_call - periodic housekeeping, called ~18.2 Hz from main thread
 * Ported from Linux signal.c - this is the critical timer/IO heartbeat
 */
static void SIGALRM_call(void)
{
    static int first = 0;
    static hitimer_t cnt200 = 0;
    static hitimer_t cnt1000 = 0;
    static volatile int running = 0;
    int retval;

    if (first == 0) {
        cnt200 = cnt1000 = pic_sys_time;
        first = 1;
    }

    /* Update terminal screen */
    if (!running) {
        if (Video && Video->update_screen) {
            running = -1;
            retval = Video->update_screen();
            running = retval ? config.term_updatefreq : 0;
        } else if (Video && Video->update_cursor) {
            Video->update_cursor();
        }
    } else if (running > 0) {
        running--;
    }

    /* Handle terminal events (keyboard, resize, etc.) */
    if (Video && Video->handle_events)
        Video->handle_events();

    /* Process keyboard input - use select() pre-check to avoid blocking
     * in slang's read() on macOS even with O_NONBLOCK set */
    if (!config.console_keyb) {
        fd_set fds;
        struct timeval tv = {0, 0};
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            keyb_client_run();
        }
    }

    /* Run serial port I/O */
    serial_run();

    /* Update CPU TSC base if using rdtsc */
    if (config.rdtsc)
        update_cputime_TSCBase();

    /* Drive the timer system - this fires PIC IRQ0 -> INT 08h */
    timer_tick();

    /* Poll for I/O events */
    io_select(fds_sigio);
    if (not_use_sigio)
        io_select(fds_no_sigio);

    /* Idle detection */
    alarm_idle();

    /* Type in pre-strokes from command line */
    if (config.pre_stroke) {
        static int count = -1;
        if (--count < 0) {
            count = type_in_pre_strokes();
            if (count < 0) count = 7;
        }
    }

    /* Per-200ms activities */
    if ((pic_sys_time - cnt200) >= (PIT_TICK_RATE / PARTIALS)) {
        cnt200 = pic_sys_time;
        printer_tick(0);
        if (config.fastfloppy)
            floppy_tick();
    }

    /* Per-second activities */
    if ((pic_sys_time - cnt1000) >= PIT_TICK_RATE) {
        cnt1000 += PIT_TICK_RATE;
        rtc_update();
    }
}

static void signal_handler(int sig, siginfo_t *si, void *uc)
{
    struct sigcontext_struct scp;
    memset(&scp, 0, sizeof(scp));

    if (sig == SIGALRM) {
        /* For cpuemu: tell emulator about the timer signal
         * e_gen_sigalrm sets sigalrm_pending in TheCPU and returns 1
         * if we should queue the SIGALRM_call housekeeping */
        if (config.cpuemu > 1) {
            if (e_gen_sigalrm(&scp)) {
                SIGNAL_save(SIGALRM_call);
            }
        } else {
            /* Non-cpuemu path: always queue housekeeping */
            SIGNAL_save(SIGALRM_call);
        }
    } else if (sig == SIGPROF) {
        /* SIGPROF is used by cpuemu for its own timing */
        if (sig_handlers[sig])
            sig_handlers[sig](&scp);
    } else {
        /* Dispatch to registered handler */
        if (sig >= 0 && sig < 32 && sig_handlers[sig])
            sig_handlers[sig](&scp);
    }
}

static void crash_handler(int sig, siginfo_t *si, void *uc)
{
    const char *signame = (sig == SIGSEGV) ? "SIGSEGV" :
                          (sig == SIGBUS) ? "SIGBUS" : "SIGNAL";
    fprintf(stderr, "\n%s at address %p\n", signame, si->si_addr);
    dbug_printf("\n*** %s at address %p ***\n", signame, si->si_addr);
    dbug_printf("lowmem_base=%p\n", lowmem_base);
    if (si->si_addr && lowmem_base) {
        long offset = (char *)si->si_addr - lowmem_base;
        dbug_printf("Offset from lowmem_base: 0x%lx (%ld)\n", offset, offset);
    }
    dbug_printf("config.ext_mem=%d EXTMEM_SIZE=0x%x\n", config.ext_mem, EXTMEM_SIZE);
    dbug_printf("LOWMEM_SIZE=0x%x HMASIZE=0x%x total=0x%x\n",
        LOWMEM_SIZE, HMASIZE, LOWMEM_SIZE + HMASIZE + EXTMEM_SIZE);
    dbug_printf("ext_mem_base=%p\n", ext_mem_base);
    /* Print CPU emulation state if available */
    {
        dbug_printf("CPU emu: trans_addr=%lx return_addr=%lx\n", trans_addr, return_addr);
        dbug_printf("CPU emu: in_vm86_emu=%d in_dpmi_emu=%d\n", in_vm86_emu, in_dpmi_emu);
    }
#ifdef __aarch64__
    if (uc) {
        ucontext_t *ucp = (ucontext_t *)uc;
        struct __darwin_arm_thread_state64 *regs = &ucp->uc_mcontext->__ss;
        dbug_printf("Host PC=%p LR=%p SP=%p FP=%p\n",
            (void *)regs->__pc, (void *)regs->__lr,
            (void *)regs->__sp, (void *)regs->__fp);
        dbug_printf("x0=%p x1=%p x2=%p x3=%p\n",
            (void *)regs->__x[0], (void *)regs->__x[1],
            (void *)regs->__x[2], (void *)regs->__x[3]);
        dbug_printf("x4=%p x5=%p x6=%p x7=%p\n",
            (void *)regs->__x[4], (void *)regs->__x[5],
            (void *)regs->__x[6], (void *)regs->__x[7]);
        dbug_printf("x8=%p x9=%p x10=%p x11=%p\n",
            (void *)regs->__x[8], (void *)regs->__x[9],
            (void *)regs->__x[10], (void *)regs->__x[11]);
        dbug_printf("x12=%p x13=%p x14=%p x15=%p\n",
            (void *)regs->__x[12], (void *)regs->__x[13],
            (void *)regs->__x[14], (void *)regs->__x[15]);
        dbug_printf("x16=%p x17=%p x18=%p x19=%p\n",
            (void *)regs->__x[16], (void *)regs->__x[17],
            (void *)regs->__x[18], (void *)regs->__x[19]);
        dbug_printf("x20=%p x21=%p x22=%p x23=%p\n",
            (void *)regs->__x[20], (void *)regs->__x[21],
            (void *)regs->__x[22], (void *)regs->__x[23]);
        dbug_printf("x24=%p x25=%p x26=%p x27=%p\n",
            (void *)regs->__x[24], (void *)regs->__x[25],
            (void *)regs->__x[26], (void *)regs->__x[27]);
        dbug_printf("x28=%p CPSR=0x%x\n",
            (void *)regs->__x[28], regs->__cpsr);
        /* Decode the faulting instruction */
        uint32_t *pc_ptr = (uint32_t *)regs->__pc;
        if (pc_ptr) {
            dbug_printf("Instruction at PC: 0x%08x\n", *pc_ptr);
            if (pc_ptr > (uint32_t *)1)
                dbug_printf("Instruction at PC-4: 0x%08x\n", *(pc_ptr-1));
        }
    }
#elif defined(__x86_64__)
    if (uc) {
        ucontext_t *ucp = (ucontext_t *)uc;
        dbug_printf("Host RIP=%p\n",
            (void *)ucp->uc_mcontext->__ss.__rip);
    }
#endif
    fflush(dbg_fd);
    /* Re-raise with default handler to get core dump */
    signal(sig, SIG_DFL);
    raise(sig);
}

void signal_init(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigfillset(&sa.sa_mask);

    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGPROF, &sa, NULL);
    sigaction(SIGIO, &sa, NULL);

    /* Crash handlers */
    {
        struct sigaction crash_sa;
        memset(&crash_sa, 0, sizeof(crash_sa));
        crash_sa.sa_sigaction = crash_handler;
        crash_sa.sa_flags = SA_SIGINFO;
        sigemptyset(&crash_sa.sa_mask);
        sigaction(SIGSEGV, &crash_sa, NULL);
        sigaction(SIGBUS, &crash_sa, NULL);
    }

    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);

    /* Unblock signals that were blocked by ioctl_init() */
    {
        sigset_t set;
        sigemptyset(&set);
        addset_signals_that_queue(&set);
        sigprocmask(SIG_UNBLOCK, &set, NULL);
    }
}

void SIG_init(void)
{
    signal_init();

    /* Set up the periodic SIGALRM timer (18.2 Hz) using ITIMER_REAL */
    struct itimerval itv;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 54925;  /* ~18.2 Hz */
    itv.it_value = itv.it_interval;
    if (setitimer(ITIMER_REAL, &itv, NULL) < 0) {
        error("SIG_init: setitimer(ITIMER_REAL) failed: %s\n", strerror(errno));
    } else {
        g_printf("SIG_init: ITIMER_REAL set, interval=%ld usec\n",
                (long)itv.it_interval.tv_usec);
    }

    /* Verify signals are not blocked */
    {
        sigset_t set;
        sigprocmask(SIG_BLOCK, NULL, &set);
        if (sigismember(&set, SIGALRM))
            error("SIG_init: WARNING - SIGALRM is BLOCKED!\n");
        else
            g_printf("SIG_init: SIGALRM is unblocked.\n");
    }
}

void SIG_close(void)
{
    struct itimerval itv;
    memset(&itv, 0, sizeof(itv));
    setitimer(ITIMER_REAL, &itv, NULL);
    setitimer(ITIMER_PROF, &itv, NULL);

    signal(SIGALRM, SIG_DFL);
    signal(SIGPROF, SIG_DFL);
    signal(SIGIO, SIG_DFL);
}

/*
 * handle_signals - process queued signal handlers in main thread
 * Called from do_periodic_stuff() in the main loop
 */
static void handle_signals_force(int force_reentry)
{
    static int in_handle_signals = 0;
    void (*handler)(void);

    if (in_handle_signals++ && !force_reentry) {
        error("BUG: handle_signals() re-entered!\n");
    }

    if (SIGNAL_head != SIGNAL_tail) {
#ifdef X86_EMULATOR
        if ((config.cpuemu > 1) && (debug_level('e') > 3))
            e_printf("EMU86: SIGNAL at %d\n", SIGNAL_head);
#endif
        signal_pending = 0;
        handler = signal_queue[SIGNAL_head].signal_handler;
        SIGNAL_head = (SIGNAL_head + 1) % MAX_SIG_QUEUE_SIZE;
        handler();

        if (SIGNAL_head != SIGNAL_tail) {
            signal_pending = 1;
        }
    }

    if (signal_pending) {
        dpmi_return_request();
    }

    in_handle_signals--;
}

void handle_signals(void)
{
    handle_signals_force(0);
}

void handle_signals_force_reentry(void)
{
    handle_signals_force(1);
}

int check_fix_fs_gs_base(unsigned char prefix)
{
    return 0;
}
