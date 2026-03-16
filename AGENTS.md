# AGENTS.md

**Version:** 1.0
**Date:** 2026-03-15
**Purpose:** Technical reference for dosdoor development (methodology in .clio/instructions.md)

---

## Project Overview

**dosdoor** is a stripped-down DOS emulator purpose-built for running BBS door games. Fork of dosemu 1.4.0 with everything unnecessary removed.

- **Language:** C (C99, ~293K lines)
- **Architecture:** DOS emulator with CPU emulation (simx86), serial I/O, filesystem mapping
- **Binary name:** `dosdoor` (was `dosemu`)
- **Philosophy:** Minimal, CPU-efficient, cross-platform door game execution

### What dosdoor Does

Emulates a DOS environment with:
- CPU emulation (JIT on x86_64, pure interpreter on ARM/macOS)
- Virtual COM1 serial port (stdin/stdout passthrough for BBS I/O)
- Host directory mapping as DOS drives (MFS)
- FreeDOS kernel boot
- DPMI for protected-mode doors

### Target Door Games

LORD, BRE, Darkness, Simpsons, OOII, Lunatix - all text-mode BBS door games communicating over serial.

---

## Quick Setup

```bash
# Linux build
./build.sh

# macOS build (ARM64 or x86_64)
cp Makefile.conf.darwin Makefile.conf
sed -i '' "s|abs_top_srcdir:=.*|abs_top_srcdir:=$(pwd)|" Makefile.conf
make

# Binary location
build/bin/dosdoor

# Run (terminal mode, serial, CPU emulation)
dosdoor -t -quiet -I 'serial { com 1 virtual }' -I 'cpuemu full' "c:\\doors\\lord.bat 1"

# Quick test (boot to DOS prompt)
dosdoor -t
```

---

## Architecture

```
dosdoor binary
├── CPU Emulation (src/emu-i386/simx86/)
│   ├── Interpreter - codegen-sim.c (portable, all platforms)
│   ├── JIT codegen - codegen-x86.c (x86/x86_64 hosts only)
│   └── Core - interp.c, cpu-emu.c, cpatch.c, fp87-sim.c
├── BIOS/Interrupts (src/base/bios/, src/base/async/)
│   ├── INT 10h - text-mode video
│   ├── INT 14h - serial (COM port)
│   ├── INT 16h - keyboard
│   ├── INT 21h - DOS services
│   └── INT 08h/1Ch - timer
├── Serial I/O (src/base/serial/)
│   ├── UART 8250/16550 emulation
│   └── Virtual COM1 -> stdin/stdout
├── Terminal (src/plugin/term/)
│   ├── ANSI/VT100 output
│   └── Keyboard input (slang)
├── Filesystem (src/dosext/mfs/)
│   ├── MFS - host directory -> DOS drives
│   └── FAT support (src/base/bios/fatfs.c)
├── DPMI (src/dosext/dpmi/)
│   └── Protected mode for doors that need it
├── Platform Abstraction
│   ├── Linux (src/arch/linux/)
│   └── macOS (src/arch/darwin/)
└── Configuration (src/base/init/)
    ├── Parser (lex/yacc config)
    └── CLI arguments
```

---

## Directory Structure

| Path | Purpose |
|------|---------|
| `src/base/` | Core emulation (BIOS, serial, PIC, timer, init) |
| `src/dosext/` | DOS extensions (DPMI, MFS filesystem, misc) |
| `src/emu-i386/simx86/` | CPU emulation (interpreter + JIT) |
| `src/arch/linux/` | Linux platform code (signals, memory mapping) |
| `src/arch/darwin/` | macOS platform code (signals, memory mapping) |
| `src/plugin/` | Plugins (terminal, keyboard, translate, commands) |
| `src/env/video/` | Video emulation (text-mode, VGA internals) |
| `src/include/` | Header files |
| `src/commands/` | Built-in DOS commands (exitemu, etc.) |
| `freedos/` | FreeDOS kernel, COMMAND.COM, config files |
| `etc/` | Default configuration files |
| `tests/` | Test scripts |
| `build/` | Build output (binary at build/bin/dosdoor) |

**Key Files:**

| File | Purpose |
|------|---------|
| `src/emu-i386/simx86/codegen-sim.c` | Pure interpreter (2792 lines) - portable CPU emulation |
| `src/emu-i386/simx86/codegen-x86.c` | JIT code generator (3286 lines) - x86_64 only |
| `src/emu-i386/simx86/interp.c` | Emulation loop dispatcher (2970 lines) |
| `src/dosext/dpmi/dpmi.c` | DPMI protected mode (4635 lines) |
| `src/dosext/mfs/mfs.c` | MFS filesystem (4284 lines) |
| `src/base/serial/ser_ports.c` | Serial port emulation |
| `src/base/async/int.c` | Interrupt handling (2338 lines) |
| `src/base/misc/stubs.c` | Stubs for stripped subsystems |
| `src/base/bios/bios.c` | BIOS initialization |
| `src/arch/darwin/signal.c` | macOS signal handling |
| `build.sh` | Build script (handles Linux configure + macOS) |
| `configure` | Linux configuration script |
| `Makefile.conf.darwin` | macOS build configuration |
| `dosdoor.spec` | RPM spec file |

---

## Code Style

**C Conventions:**

- C99 standard (`-std=c99` not explicitly set, but code is C99-compatible)
- `-fgnu89-inline` for legacy `extern inline` compatibility
- Brief comments describing what, not why (git history handles why)
- No dramatic annotations ("CRITICAL FIX", "IMPORTANT", etc.)
- Guard platform-specific code with `#ifdef __linux__`, `#ifdef __APPLE__`, etc.
- Guard architecture-specific code with `#if defined(__x86_64__) || defined(__i386__)`

**Memory/Pointer Patterns (CRITICAL):**

- `lowmem_base` - base address of emulated DOS low memory in host address space
- `MK_FP32(seg, off)` - converts seg:off to host pointer (adds lowmem_base)
- `SEGOFF2LINEAR(seg, off)` - converts seg:off to DOS linear address (NO lowmem_base)
- `SEG_ADR(type, seg, off)` - host pointer via lowmem_base
- `SEG_ADR_LINEAR(seg, off)` - DOS linear address (for e_invalidate, JIT cache)
- **NEVER cast SEGOFF2LINEAR to a pointer for dereferencing** - it's a DOS address, not a host pointer
- `e_invalidate()` expects DOS linear addresses, NOT lowmem_base-based pointers

**Platform Guards:**

```c
// Architecture
#if defined(__x86_64__) || defined(__i386__)
  // x86 inline asm here
#else
  // C fallback for ARM/other
#endif

// OS
#ifdef __linux__
  #include <sys/kd.h>
#endif

#ifdef __APPLE__
  // macOS-specific code
#endif
```

**Debug Logging:**

```c
// Standard pattern
g_printf("MODULE: message, value=%d\n", value);
D_printf("MODULE: debug detail\n");

// Error conditions
error("MODULE: something went wrong: %s\n", strerror(errno));
```

---

## Build System

**Plain Makefile** with lightweight `configure` script. No autotools, no CMake.

### Linux Build

```bash
./build.sh
# Or manually:
./configure --enable-cpuemu --disable-net --without-x --disable-sbemu \
    --disable-mitshm --without-vidmode --disable-aspi --without-gpm \
    --without-alsa --without-sndfile --disable-dlplugins
sed -i 's/CFLAGS += $(XXXCFLAGS)/CFLAGS += -fgnu89-inline $(XXXCFLAGS)/' Makefile.conf
make
```

### macOS Build

```bash
cp Makefile.conf.darwin Makefile.conf
sed -i '' "s|abs_top_srcdir:=.*|abs_top_srcdir:=$(pwd)|" Makefile.conf
echo '#!/bin/sh' > config.status && chmod +x config.status
make
```

### Build Dependencies

- gcc or clang
- make
- flex, bison (parser generation)
- libslang2-dev / slang-devel (terminal I/O)

### Build Output

- `build/bin/dosdoor` - main binary
- `build/commands/` - built-in DOS commands

---

## Testing

### Runtime Testing

```bash
# Boot to DOS prompt (terminal mode)
dosdoor -t

# With CPU emulation and serial
dosdoor -t -I 'serial { com 1 virtual }' -I 'cpuemu full'

# Run a door game
dosdoor -t -quiet -I 'serial { com 1 virtual }' -I 'cpuemu full' \
    "c:\\doors\\lord.bat 1"
```

### Door Game Test Matrix

| Door | Status | Notes |
|------|--------|-------|
| LORD 3.50 | Working | Needs 52-line DOOR.SYS, /DREW flag for serial mode |
| BRE v0.988 | Working | Full game flow through serial |
| Darkness | Working | Launches, checks serial, clean exit |
| OOII | Working | Runs daily maintenance, clean exit |
| Simpsons v1.2 | Working | Title screen via serial |
| Lunatix | Not tested | Not available |

### Platform Test Matrix

| Platform | CPU Mode | Status |
|----------|----------|--------|
| Linux x86_64 | JIT full | Working |
| macOS ARM64 | Interpreter (fullsim) | Working |
| Linux ARM64 | Not tested | Target |
| macOS x86_64 | Not tested | Target |

### Automated Tests

```bash
# Test scripts
tests/doortest.py    # Python door game test harness
tests/doortest.sh    # Shell-based test runner
tests/fixtures/      # Test data files
```

### Pre-Commit Checks

```bash
# Verify build succeeds
./build.sh

# Verify binary runs
build/bin/dosdoor --version

# Check for undefined symbols (Linux)
nm -u build/bin/dosdoor | grep -v ' U ' | head
```

---

## Commit Format

```
type(scope): brief description
```

**Types:** `feat`, `fix`, `refactor`, `docs`, `test`, `chore`

**Git workflow:**
- Squash commits before pushing
- Do NOT push to origin without asking
- Clean, descriptive commit messages
- Use `release.sh` for tagged releases

**Version Tagging (MANDATORY):**

Tags MUST follow the `YYYYMMDD.N` format. No exceptions.

- Format: `YYYYMMDD.N` where N starts at 0 and increments for multiple releases on the same day
- Examples: `20260316.0`, `20260316.1`, `20260315.2`
- **NEVER use semver** (`1.4.0.1`, `v1.0.0`, `1.0`, etc.) - it is wrong for this project
- The internal version string in source (e.g. `VERSION` file, dosemu version) is independent of git tags; do not confuse them
- Use `release.sh YYYYMMDD.N` to create a release - it handles tagging correctly

**Pre-Commit Checklist:**
-  Build succeeds on target platform
-  No `ai-assisted/` files staged (`git reset HEAD ai-assisted/`)
-  Commit message explains what changed
-  No TODO/FIXME comments (finish the work)

---

## CI/CD

**GitHub Actions** workflow at `.github/workflows/build.yml`:
- Triggers on push to `main`, tags, and PRs
- `build-linux` job: compile on Ubuntu, verify binary
- `build-rpm` job: build RPM package
- `release` job: create GitHub release on tag push with binary, RPM, and source tarball

---

## Common Patterns

### Platform-Conditional Code

The codebase has two platform layers:
- `src/arch/linux/` - Linux (signals via sigcontext, mmap with MAP_FIXED)
- `src/arch/darwin/` - macOS (signals via ucontext, mmap without alias-at-0)

Key difference: Linux x86_64 uses alias-at-0 memory mapping (DOS low memory mapped at address 0), macOS/ARM uses `lowmem_base` offset for all DOS memory access.

### DOS Memory Access

```c
// Host pointer to DOS memory (for reading/writing)
char *ptr = MK_FP32(seg, off);          // seg:off -> host pointer
char *ptr = SEG_ADR(char, es, bx);      // register-based

// DOS linear address (for JIT cache, e_invalidate)
unsigned addr = SEGOFF2LINEAR(seg, off); // seg:off -> linear
unsigned addr = SEG_ADR_LINEAR(es, bx);  // register-based
```

### Adding Stubs for Stripped Subsystems

When removing subsystems, add function stubs to `src/base/misc/stubs.c` to satisfy linker references from code that conditionally calls into removed subsystems.

### Pre-assembled x86 Data

x86 `.S` assembly files are pre-assembled on Linux and embedded as C byte arrays for ARM/macOS portability:
- `src/base/bios/bios_data.c` (BIOS code)
- `src/dosext/dpmi/dpmisel_data.c` (DPMI selectors)
- `src/base/bios/fatfs_boot_data.c` (FAT boot sector)

---

## Known Issues / Gotchas

1. **SEGOFF2LINEAR is NOT a host pointer** - casting it to `char*` for dereferencing crashes on macOS/ARM where lowmem_base != 0. Use `MK_FP32()` instead.

2. **e_invalidate() needs DOS linear addresses** - on x86_64 Linux with alias-at-0, JIT cache keys are DOS linear addresses. Passing lowmem_base-based pointers silently fails to invalidate.

3. **macOS stdin blocking** - `read(stdin)` can block on macOS even with `O_NONBLOCK`. The `SIGALRM_call` in `darwin/signal.c` wraps `keyb_client_run()` with a `select()` pre-check.

4. **LORD needs 52-line DOOR.SYS** - short format causes range check errors. Also needs `/DREW` flag for serial/remote mode.

5. **find_file() NULL dereference** - in `mfs.c`, `find_file()` can receive NULL `doserrno` parameter. Fixed with NULL check.

6. **JIT cache staleness** - disk reads via INT 13h go through `lowmem_base` but JIT cache is keyed by DOS linear addresses. After disk reads, call `e_invalidate()` with the DOS linear address.

---

## Anti-Patterns (What NOT To Do)

| Anti-Pattern | Why It's Wrong | What To Do |
|--------------|----------------|------------|
| Cast SEGOFF2LINEAR to pointer | Crashes on non-alias-at-0 platforms | Use MK_FP32() for host pointers |
| Pass lowmem_base addresses to e_invalidate | Silently fails to invalidate JIT cache | Use SEG_ADR_LINEAR for DOS linear addresses |
| Add external runtime dependencies | Violates zero-deps goal | Keep it self-contained |
| Use autotools/CMake | Project uses plain Makefile + configure | Edit Makefile/configure directly |
| Busy-wait polling loops | CPU efficiency is primary goal | Use adaptive sleep, event-driven I/O |
| Skip platform guards on new code | Breaks cross-platform builds | Always guard arch/OS-specific code |
| Defer bugs | Project policy: fix bugs as found | Fix immediately |
| Add unnecessary subsystems back | Stripped for a reason | Check PRD section 2.4 before adding |

---

## Quick Reference

**Build:**
```bash
./build.sh                    # Full build (Linux or macOS)
make                          # Rebuild after changes
make -C src/base/misc         # Rebuild single directory
```

**Run:**
```bash
build/bin/dosdoor -t          # Boot to DOS prompt
build/bin/dosdoor --version   # Version info
```

**Search Code:**
```bash
git grep "pattern" src/       # Search source
grep -rn "pattern" src/include/  # Search headers
```

**Release:**
```bash
./release.sh 20260315.1       # Tag a release (format: YYYYMMDD.N, NEVER semver)
git push && git push --tags   # Push release
```

**Git:**
```bash
git status
git diff
git log --oneline -10
git add -A && git commit -m "type(scope): description"
```

---

*For project methodology and workflow, see .clio/instructions.md*
*For project goals and requirements, see .clio/PRD.md*
