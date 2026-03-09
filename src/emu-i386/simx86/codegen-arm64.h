/*
 * ARM64 Cached Interpreter Backend for simx86
 *
 * This header declares the ARM64 cached interpreter initialization.
 * The cached interpreter captures IGen operations (like the x86 JIT)
 * but replays them through sim-equivalent C code instead of emitting
 * native ARM64 instructions.
 *
 * (C) Copyright 2026 the "DoorEmu Development Team"
 */

#ifndef _EMU86_CODEGEN_ARM64_H
#define _EMU86_CODEGEN_ARM64_H

#if defined(__aarch64__) || defined(__arm__)

/* Initialization - called from InitGen() in codegen-sim.c */
void InitGen_arm64(void);

#endif /* __aarch64__ || __arm__ */

#endif /* _EMU86_CODEGEN_ARM64_H */
