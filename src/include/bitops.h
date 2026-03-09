/*
 * Portable bitops.h - works on both x86 and ARM64
 * Original x86 inline asm by Linus Torvalds, find_bit by J. Lawrence Stephan
 * C fallbacks for non-x86 platforms
 */

#ifndef _ASM_BITOPS_H
#define _ASM_BITOPS_H

static int find_bit(unsigned int word);
static int find_bit_r(unsigned int word);
static int set_bit(int nr, void * addr);
static int clear_bit(int nr, void * addr);
static int change_bit(int nr, void * addr);
static int test_bit(int nr, void * addr);
static int pic0_to_emu(char flags);

#define ADDR (*(volatile long *) addr)

#if defined(__x86_64__) || defined(__i386__)

/* x86 optimized versions */
static __inline__ int find_bit(unsigned int word) {
    int result = -1;
    __asm__("bsfl %2,%0" :"=r" (result) :"0" (result), "r" (word));
    return result;
}

static __inline__ int find_bit_r(unsigned int word) {
    int result = -1;
    __asm__("bsrl %2,%0" :"=r" (result) :"0" (result), "r" (word));
    return result;
}

static __inline__ int pic0_to_emu(char flags) {
    int result;
    __asm__ __volatile__("movzbl %1,%0\n\t"
        "shll $13, %0\n\t" "sarw $7, %w0\n\t" "shrl $5, %0 "
        :"=r"(result):"q"(flags));
    return result;
}

static __inline__ int emu_to_pic0(int flags) {
    __asm__ __volatile__("shll $6,%0\n\t"
        "shlw $7, %w0\n\t" "shrl $14, %0 "
        :"=r"(flags):"0"(flags));
    return flags;
}

static __inline__ int change_bit(int nr, void *addr) {
    int oldbit;
    __asm__ __volatile__("btcl %2,%1\n\tsbbl %0,%0"
        :"=r"(oldbit), "=m"(ADDR) :"r"(nr));
    return oldbit;
}

static __inline__ int set_bit(int nr, void *addr) {
    int oldbit;
    __asm__ __volatile__("btsl %2,%1\n\tsbbl %0,%0"
        :"=r"(oldbit), "=m"(ADDR) :"r"(nr));
    return oldbit;
}

static __inline__ int clear_bit(int nr, void *addr) {
    int oldbit;
    __asm__ __volatile__("btrl %2,%1\n\tsbbl %0,%0"
        :"=r"(oldbit), "=m"(ADDR) :"r"(nr));
    return oldbit;
}

static __inline__ int test_bit(int nr, void *addr) {
    int oldbit;
    __asm__ __volatile__("btl %2,%1\n\tsbbl %0,%0"
        :"=r"(oldbit) :"m"(ADDR), "r"(nr));
    return oldbit;
}

#else /* non-x86: portable C implementations */

static inline int find_bit(unsigned int word) {
    if (!word) return -1;
    int n = 0;
    while (!(word & 1)) { word >>= 1; n++; }
    return n;
}

static inline int find_bit_r(unsigned int word) {
    if (!word) return -1;
    int n = 31;
    while (!(word & (1u << 31))) { word <<= 1; n--; }
    return n;
}

static inline int pic0_to_emu(char flags) {
    unsigned int f = (unsigned char)flags;
    f <<= 13;
    f = (f & 0xFFFF0000u) | ((unsigned short)((short)(unsigned short)f >> 7));
    f >>= 5;
    return (int)f;
}

static inline int emu_to_pic0(int flags) {
    flags <<= 6;
    flags = (flags & (int)0xFFFF0000u) | ((unsigned short)((unsigned short)flags << 7));
    flags = (unsigned)flags >> 14;
    return flags;
}

static inline int change_bit(int nr, void *addr) {
    volatile unsigned long *p = (volatile unsigned long *)addr;
    unsigned long mask = 1UL << (nr & (sizeof(long)*8 - 1));
    int oldbit = (p[nr / (sizeof(long)*8)] & mask) ? -1 : 0;
    p[nr / (sizeof(long)*8)] ^= mask;
    return oldbit;
}

static inline int set_bit(int nr, void *addr) {
    volatile unsigned long *p = (volatile unsigned long *)addr;
    unsigned long mask = 1UL << (nr & (sizeof(long)*8 - 1));
    int oldbit = (p[nr / (sizeof(long)*8)] & mask) ? -1 : 0;
    p[nr / (sizeof(long)*8)] |= mask;
    return oldbit;
}

static inline int clear_bit(int nr, void *addr) {
    volatile unsigned long *p = (volatile unsigned long *)addr;
    unsigned long mask = 1UL << (nr & (sizeof(long)*8 - 1));
    int oldbit = (p[nr / (sizeof(long)*8)] & mask) ? -1 : 0;
    p[nr / (sizeof(long)*8)] &= ~mask;
    return oldbit;
}

static inline int test_bit(int nr, void *addr) {
    volatile unsigned long *p = (volatile unsigned long *)addr;
    unsigned long mask = 1UL << (nr & (sizeof(long)*8 - 1));
    return (p[nr / (sizeof(long)*8)] & mask) ? -1 : 0;
}

#endif /* x86 vs non-x86 */

#endif /* _ASM_BITOPS_H */
