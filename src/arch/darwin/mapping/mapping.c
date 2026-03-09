/*
 * mapping.c - macOS memory mapping for dosdoor
 * Simplified version using mmap/munmap
 */
#include "config.h"
#include "emu.h"
#include "mapping.h"
#include "utilities.h"
#include "memory.h"

#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef MAP_NORESERVE
#define MAP_NORESERVE 0
#endif

/* macOS with Hardened Runtime doesn't allow W+X (write+execute) pages.
   Since we use pure CPU emulation (interpreter), we never actually
   execute code from emulated memory - strip PROT_EXEC when combined
   with PROT_WRITE. */
static inline int darwin_sanitize_prot(int prot) {
    if ((prot & PROT_WRITE) && (prot & PROT_EXEC)) {
        prot &= ~PROT_EXEC;
    }
    return prot;
}

/* The base address of the low memory image (0--1MB+64K) */
char *lowmem_base;

void *alias_mapping(int cap, void *target, size_t mapsize, int protect, void *source)
{
    void *addr;
    /* On macOS, offset low addresses through lowmem_base */
    if (target && target != (void*)-1 && target != MAP_FAILED) {
        if ((uintptr_t)target < 0x110000 && lowmem_base) {
            target = lowmem_base + (uintptr_t)target;
        }
    } else {
        target = NULL;
    }
    if (source && source != (void*)-1 && source != MAP_FAILED) {
        if ((uintptr_t)source < 0x110000 && lowmem_base) {
            source = lowmem_base + (uintptr_t)source;
        }
    } else {
        source = NULL;
    }
    if (target) {
        addr = mmap(target, mapsize, darwin_sanitize_prot(protect), MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    } else {
        addr = mmap(NULL, mapsize, darwin_sanitize_prot(protect), MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }
    if (addr == MAP_FAILED) return MAP_FAILED;
    if (source && source != MAP_FAILED && (protect & (PROT_READ | PROT_WRITE))) {
        memcpy(addr, source, mapsize);
    }
    return addr;
}

void *mmap_mapping(int cap, void *target, size_t mapsize, int protect, off_t source)
{
    int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
    if (target && target != (void*)-1 && target != MAP_FAILED) {
        /* On macOS with CPU emulation, low addresses (< 1GB) are not directly
           mappable. If the target is a low address that would be in DOS memory
           space, offset it via lowmem_base instead. */
        if ((uintptr_t)target < 0x110000 && lowmem_base) {
            target = lowmem_base + (uintptr_t)target;
        }
        flags |= MAP_FIXED;
    } else {
        target = NULL;
    }
    void *addr = mmap(target, mapsize, darwin_sanitize_prot(protect), flags, -1, 0);
    if (addr == MAP_FAILED) {
        error("mmap_mapping failed: %s (target=%p size=%zu prot=%d)\n",
              strerror(errno), target, mapsize, protect);
        return MAP_FAILED;
    }
    return addr;
}

void *mremap_mapping(int cap, void *source, size_t old_size, size_t new_size,
    unsigned long flags, void *target)
{
    /* macOS doesn't have mremap. Allocate new, copy, unmap old */
    int prot = PROT_READ | PROT_WRITE;
    void *new_addr;
    if (target && (flags & 2)) { /* MREMAP_FIXED */
        new_addr = mmap(target, new_size, prot, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    } else {
        new_addr = mmap(NULL, new_size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }
    if (new_addr == MAP_FAILED) return MAP_FAILED;
    size_t copy_size = old_size < new_size ? old_size : new_size;
    memcpy(new_addr, source, copy_size);
    munmap(source, old_size);
    return new_addr;
}

void *extended_mremap(void *addr, size_t old_len, size_t new_len,
    int flags, void *new_addr)
{
    return mremap_mapping(0, addr, old_len, new_len, flags, new_addr);
}

int mprotect_mapping(int cap, void *addr, size_t mapsize, int protect)
{
    return mprotect(addr, mapsize, darwin_sanitize_prot(protect));
}

int munmap_mapping(int cap, void *addr, size_t mapsize)
{
    return munmap(addr, mapsize);
}

void mapping_init(void)
{
}

void mapping_close(void)
{
}

char *decode_mapping_cap(int cap)
{
    static char buf[64];
    snprintf(buf, sizeof(buf), "cap=0x%x", cap);
    return buf;
}

int open_mapping(int cap)
{
    return 0;
}

void close_mapping(int cap)
{
}

void *alloc_mapping(int cap, size_t mapsize, off_t target)
{
    void *addr;
    if (cap & MAPPING_INIT_LOWRAM) {
        /* Allocate extra guard page to catch buffer overflows */
        size_t total = mapsize + 4096;
        addr = mmap_mapping(cap, NULL, total, PROT_READ | PROT_WRITE, 0);
        if (addr != MAP_FAILED) {
            mprotect((char*)addr + mapsize, 4096, PROT_NONE);
            *(void **)(&lowmem_base) = addr;
        }
    } else {
        addr = mmap_mapping(cap, NULL, mapsize, PROT_READ | PROT_WRITE, 0);
    }
    return addr;
}

void free_mapping(int cap, void *addr, size_t mapsize)
{
    munmap(addr, mapsize);
}

void *realloc_mapping(int cap, void *addr, size_t oldsize, size_t newsize)
{
    return mremap_mapping(cap, addr, oldsize, newsize, 0, NULL);
}
