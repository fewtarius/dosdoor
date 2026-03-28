# FreeDOS Binary Sources

The FreeDOS binaries included with dosdoor are GPL-licensed components
originally distributed with dosemu 1.4.0. Source code is available from
the FreeDOS project:

## kernel.sys - FreeDOS Kernel
- License: GPL-2.0-or-later
- Source: https://github.com/FDOS/kernel

## command.com - FreeCOM (FreeDOS Command Interpreter)
- License: GPL-2.0-or-later
- Source: https://github.com/FDOS/freecom

## dosemu/ directory
The files in `dosemu/` are DOS-side utilities built from the dosdoor
source tree (`src/commands/`). They are part of the dosdoor project
and licensed under GPL-2.0-or-later.

- `emufs.sys` - DOS filesystem driver (pre-assembled from src/commands/emufs.S)
- `ems.sys` - EMS memory driver (pre-assembled from src/commands/ems.S)
- `fossil.com` - FOSSIL serial driver (pre-assembled from src/commands/fossil.S)
- `exitemu.com` - Exit emulator command (built from src/commands/exitemu.S)
- `generic.com` - Generic command handler (built from src/commands/)
- `lredir.com` - Drive redirect utility (built from src/commands/lredir.c)
- `system.com` - System utility (pre-assembled)
- `unix.com` - Unix integration (built from src/commands/unix.c)
