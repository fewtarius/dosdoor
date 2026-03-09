/* 
 * (C) Copyright 1992, ..., 2007 the "DOSEMU-Development-Team".
 *
 * for details see file COPYING.DOSEMU in the DOSEMU distribution
 */

#include "codegen-sim.h"
#ifdef HOST_ARCH_X86
#include "codegen-x86.h"
#endif
#if defined(__aarch64__) || defined(__arm__)
#include "codegen-arm64.h"
#endif
