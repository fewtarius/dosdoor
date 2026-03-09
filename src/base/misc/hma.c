/* 
 * (C) Copyright 1992, ..., 2007 the "DOSEMU-Development-Team".
 *
 * for details see file COPYING.DOSEMU in the DOSEMU distribution
 */

/*
 * Robert Sanders, started 3/1/93
 *
 * Hans Lermen, moved 'mapping' to generic mapping drivers, 2000/02/02
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "memory.h"
#include "emu.h"
#include "hma.h"
#include "mapping.h"
#include "bios.h"
#include "utilities.h"
#include "dos2linux.h"
#include "cpu-emu.h"

#define HMAAREA 0x100000

unsigned char *ext_mem_base = NULL;

void HMA_MAP(int HMA)
{
#ifdef __APPLE__
  /* macOS: we can't use mmap aliasing because our lowmem region is one
   * contiguous allocation. Instead, use memcpy to simulate HMA wrapping.
   * A20 off (HMA=0): HMA mirrors low memory (copy 0x0000-0xFFFF to 0x100000)
   * A20 on  (HMA=1): HMA is independent (copy saved HMA content back) */
  static char hma_saved[HMASIZE];
  static int hma_saved_valid = 0;
  unsigned char *hma_ptr = (unsigned char *)LOWMEM(HMAAREA);
  unsigned char *low_ptr = (unsigned char *)LOWMEM(0);

  x_printf("HMA_MAP macOS: HMA=%d hma_ptr=%p low_ptr=%p\n", HMA, hma_ptr, low_ptr);

  if (HMA) {
    /* A20 on: restore HMA's own content */
    if (hma_saved_valid) {
      memcpy(hma_ptr, hma_saved, HMASIZE);
    }
    /* else: first time, HMA keeps whatever is there (zeros initially) */
  } else {
    /* A20 off: save HMA content, then copy low memory to HMA */
    if (hma_saved_valid) {
      /* already have saved data, save current state */
      memcpy(hma_saved, hma_ptr, HMASIZE);
    }
    memcpy(hma_ptr, low_ptr, HMASIZE);
    hma_saved_valid = 1;
  }
#else
  void *ipc_return;
  /* Note: MAPPING_HMA is magic, dont be confused by src==dst==HMAAREA here */
  off_t src = HMA ? HMAAREA : 0;
  x_printf("Entering HMA_MAP with HMA=%d\n", HMA);

  if (munmap_mapping(MAPPING_HMA, (void *)HMAAREA, HMASIZE) < 0) {
    x_printf("HMA: Detaching HMAAREA unsuccessful: %s\n", strerror(errno));
    leavedos(48);
  }
  x_printf("HMA: detached at %#x\n", HMAAREA);

  ipc_return = mmap_mapping(MAPPING_HMA, (void *)HMAAREA, HMASIZE,
    PROT_READ | PROT_WRITE | PROT_EXEC, src);
  if (ipc_return == MAP_FAILED) {
    x_printf("HMA: Mapping HMA to HMAAREA %#x unsuccessful: %s\n",
	       HMAAREA, strerror(errno));
    leavedos(47);
  }
  x_printf("HMA: mapped to %p\n", ipc_return);
#endif
}

void
set_a20(int enableHMA)
{
  if (a20 == enableHMA) {
    g_printf("WARNING: redundant %s of A20!\n", enableHMA ? "enabling" :
	  "disabling");
    return;
  }

  /* to turn the A20 on, one must unmap the "wrapped" low page, and
   * map in the real HMA memory. to turn it off, one must unmap the HMA
   * and make FFFF:xxxx addresses wrap to the low page.
   */
  HMA_MAP(enableHMA);

  a20 = enableHMA;
}

void HMA_init(void)
{
  /* initially, no HMA */
  HMA_MAP(0);
  a20 = 0;
  if (config.ext_mem) {
#ifdef __APPLE__
    /* macOS: extended memory is part of the contiguous lowmem_base allocation.
       ext_mem_base points into it at the right offset. */
    ext_mem_base = (unsigned char *)lowmem_base + LOWMEM_SIZE + HMASIZE;
    x_printf("Ext.Mem of size 0x%x at %p (contiguous with lowmem)\n", EXTMEM_SIZE, ext_mem_base);
#else
    ext_mem_base = mmap_mapping(MAPPING_EXTMEM | MAPPING_SCRATCH, (void*)-1,
      EXTMEM_SIZE, PROT_READ | PROT_WRITE, 0);
    x_printf("Ext.Mem of size 0x%x at %p\n", EXTMEM_SIZE, ext_mem_base);
#endif
    memcheck_addtype('x', "Extended memory (HMA+XMS)");
    memcheck_reserve('x', LOWMEM_SIZE, HMASIZE + EXTMEM_SIZE);
  }
}


void
hma_exit(void)
{
}

void extmem_copy(char *dst, char *src, unsigned long len)
{
  unsigned long slen, dlen, clen, copied = 0;
  unsigned char *s, *d, *edge = (unsigned char *)(LOWMEM_SIZE + HMASIZE);

  while ((clen = len - copied) > 0) {
    slen = clen;
    s = src + copied;
    if (s >= edge)
      s += ext_mem_base - edge;
    else if (s + slen > edge)
      slen = edge - s;

    dlen = clen;
    d = dst + copied;
    if (d >= edge)
      d += ext_mem_base - edge;
    else if (d + dlen > edge)
      dlen = edge - d;

    clen = min(slen, dlen);
    x_printf("INT15: copy 0x%lx bytes from %p to %p%s\n",
      clen, s, d, clen != len ? " (split)" : "");
    memmove_dos2dos(d, s, clen);
    if (d < edge)
      e_invalidate(d, clen);
    copied += clen;
  }
}
