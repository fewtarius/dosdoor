/*
 * ARM64 Backend for simx86
 *
 * On ARM64, we use the pure software interpreter (codegen-sim).
 * This file provides InitGen_arm64() called after the sim backend
 * is initialized, as a hook for ARM64-specific setup.
 *
 * The sim interpreter is adequate for BBS door games which spend
 * most of their time waiting for user input. CPU efficiency comes
 * from clock speed throttling and idle detection rather than
 * execution speed.
 */

#include <stddef.h>
#include <stdlib.h>
#include "emu86.h"
#include "codegen-sim.h"
#include "trees.h"

#if defined(__aarch64__) || defined(__arm__)

#include "codegen-arm64.h"

/*
 * ARM64-specific initialization.
 * Called after the sim backend (Gen_sim, AddrGen_sim, CloseAndExec_sim)
 * has been set up by InitGen() in codegen-sim.c.
 *
 * Currently a no-op. Future uses:
 * - Enable tree caching for decoded instruction replay
 * - Set up ARM64-specific throttling parameters
 */
void InitGen_arm64(void)
{
	/* ARM64 backend initialized (using sim interpreter) */
	if (debug_level('e') > 0)
		dbug_printf("ARM64: using software interpreter\n");
}

#else /* !__aarch64__ && !__arm__ */

/* Stub for non-ARM builds */
void InitGen_arm64(void)
{
}

#endif
