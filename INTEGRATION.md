# dosdoor - PhotonBBS Integration Guide

## Overview

dosdoor is a drop-in replacement for dosemu in BBS doorexec scripts. The
command-line interface is backward compatible - replace `dosemu` with `dosdoor`
and everything works.

## Quick Migration

```bash
# Before (dosemu)
dosemu -t -quiet -I "serial { com 1 virtual }" "c:\\doors\\lord.bat 1"

# After (dosdoor)
dosdoor -t -quiet -I "serial { com 1 virtual }" "c:\\doors\\lord.bat 1"
```

The `DOSEMU_HOGTHRESHOLD` environment variable is accepted for backward
compatibility (dosdoor has built-in idle detection).

## Directory structure

dosdoor uses the same directory layout as dosemu:

```
$HOME/.dosemu/
├── drives/
│   └── c/             DOS C:\ drive mapping
│       ├── doors/     Door game directories
│       └── nodeinfo/  Per-node drop files
└── freedos/           FreeDOS kernel + utilities
```

## doorexec scripts

Each door game has a launcher script called with:
```
doorexec/<name>.sh <bbs_home> <node_number> <username>
```

### Example: LORD

```bash
#!/bin/bash
export HOME="$1"
export TERM="ansi"

dosdoor -t -quiet -I "serial { com 1 virtual }" "c:\\doors\\lord.bat $2" 2>/dev/null
reset
```

### Example: BRE

```bash
#!/bin/bash
export HOME="$1"
export TERM="ansi"

dosdoor -t -quiet -I "serial { com 1 virtual }" "c:\\doors\\bre.bat $2" 2>/dev/null
reset
```

## Drop files

BBS software creates drop files before invoking doorexec scripts:

- **DORINFO1.DEF** - Most games (Darkness, Simpsons)
- **DOOR.SYS** - 52-line format (LORD requires this)
- Game-specific utilities (SRDOOR for BRE, OOINFO for OOII)

Drop files go in `C:\NODEINFO\<node>\` on the DOS drive.

## Per-door configuration

| Door | Extra setup | Notes |
|------|-------------|-------|
| LORD | DOOR.SYS + NODE*.DAT | Requires /DREW flag for serial mode |
| BRE | SRDOOR setup file | Uses setup.N for node config |
| OOII | OOINFO utility | Cleanup oonode.dat on exit |
| Lunatix | Template files per-node | Copy lunatix.N, linkto.N |
| Darkness | DORINFO copy to game dir | Standard DORINFO handling |
| Simpsons | Standard DORINFO | No extra setup |

## I/O flow

```
BBS user (SSH/telnet) <-> BBS software <-> doorexec script
                                                |
                                            dosdoor -t
                                                |
                                    stdin/stdout <-> virtual COM1
                                                |
                                        DOS serial API
                                                |
                                      Door game executable
```

## Node isolation

Each BBS node runs its own dosdoor process. Isolation is through:

1. **Separate drop files:** `C:\NODEINFO\1\`, `C:\NODEINFO\2\`, etc.
2. **Node number argument:** Passed via batch file parameter
3. **Process isolation:** Each instance is a separate OS process

The game files directory is shared across nodes. Node-specific state goes
in the drop file directories.

## Differences from dosemu

| Feature | dosemu | dosdoor |
|---------|--------|---------|
| Binary name | `dosemu.bin` | `dosdoor` |
| Sound | SB16, MIDI, speaker | Removed |
| Graphics | VGA, X11, SDL | Text mode only |
| Mouse/networking | Full | Removed |
| CPU usage | Needs HOGTHRESHOLD | Built-in idle detection |
| Platforms | Linux x86 | Linux x86_64, macOS ARM64 |
| CPU emulation | vm86 + software | Software only (simx86) |

## Troubleshooting

**Door hangs on launch:** Verify COM1 is configured with
`-I "serial { com 1 virtual }"`. Check batch file uses DOS backslashes.

**No output:** Export `TERM=ansi`. Try without `-quiet` to see boot messages.

**Exits immediately:** Check that drop files exist in
`~/.dosemu/drives/c/nodeinfo/<node>/`. Verify batch file ends with `exitemu`.

## Batch migration

To update all doorexec scripts at once:

```bash
for script in doorexec/*.sh; do
    sed -i 's/dosemu -t/dosdoor -t/g' "$script"
    sed -i '/DOSEMU_HOGTHRESHOLD/d' "$script"
done
```
