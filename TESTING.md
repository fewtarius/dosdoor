# Testing Door Games with dosdoor

## Overview

dosdoor runs BBS door games by emulating a DOS environment with virtual
serial port (COM1). Games communicate through the serial port, which maps
to the calling terminal's stdin/stdout - the same way a BBS presents
door games to callers.

## Build

```bash
# macOS (ARM64 or x86_64)
cp Makefile.conf.darwin Makefile.conf
make

# Linux (x86_64 or ARM64)
./configure
make
```

Binary: `build/bin/dosdoor`

## Test Environment Setup

The test environment requires three directories:

### 1. Library directory (libdir)

Contains the FreeDOS boot files and keymap data. Create from the repo's
`freedos/` directory:

```bash
mkdir -p /tmp/dosemu_lib/drive_z
cp -r freedos/* /tmp/dosemu_lib/drive_z/
mkdir -p /tmp/dosemu_lib/drives/c
mkdir -p /tmp/dosemu_lib/drives/d
cp -r etc/keymap /tmp/dosemu_lib/keymap
```

### 2. Configuration directory

```bash
mkdir -p /tmp/dosemu_etc
```

Create `/tmp/dosemu_etc/global.conf`:

```
$_hdimage = "drives/*"
$_floppy_a = ""
$_floppy_b = ""
$_com1 = "virtual"
$_com2 = ""
$_com3 = ""
$_com4 = ""
$_mouse = ""
$_mouse_dev = ""
$_mouse_flags = ""
$_speaker = ""
$_joy_device = ""
$_layout = "us"
```

Create `/tmp/dosemu_etc/dosemu.conf` (adjust the `$_hdimage` path):

```
$_hdimage = "/path/to/your/drive_c"
$_floppy_a = ""
$_floppy_b = ""
$_com1 = ""
$_com2 = ""
$_com3 = ""
$_com4 = ""
$_mouse = ""
$_mouse_dev = ""
$_mouse_flags = ""
$_sound = (off)
$_sb_base = (0)
$_speaker = ""
$_joy_device = ""
$_pci = (off)
$_layout = "us"
$_rawkeyboard = (off)
$_console = (0)
$_graphics = (0)
```

### 3. DOS Drive C

This is the virtual hard disk seen by DOS programs as `C:\`. Minimum
structure for door game testing:

```
drive_c/
├── doors/
│   ├── lord/           # LORD game files
│   │   ├── LORD.EXE
│   │   ├── lord.bat
│   │   └── ...
│   ├── bre/            # BRE game files
│   │   ├── BRE.EXE
│   │   ├── srdoor.exe
│   │   ├── bre.bat
│   │   └── ...
│   ├── darkness/       # Darkness game files
│   ├── ooii/           # Operation: Overkill II
│   └── simp/           # The Simpsons door
├── nodeinfo/
│   └── 1/              # Node 1 drop files
│       ├── DOOR.SYS
│       └── DORINFO1.DEF
└── dosemu/             # (optional) path set in AUTOEXEC.BAT
```

### Drop Files

Door games read "drop files" to get caller information. Two formats
are used:

**DORINFO1.DEF** (13 lines):

```
BBS Name
SysOp First
SysOp Last
COM1:
38400 BAUD,N,8,1
1
Caller First
Caller Last

1
100
480
```

Fields: BBS name, sysop first/last name, COM port, baud string, node
number (0=local), caller first/last name, location (blank), ANSI (0=no,
1=yes), security level, time remaining (minutes).

**DOOR.SYS** (52 lines):

```
COM1:
38400
8
1
38400
Y
Y
Y
Y
Caller Name

555-555-1234
555-555-1234
PASSWORD
100
1
03-15-26
38400
480
GR
25
N

03-15-26
1
Y
0
0
0
999999
03-15-26
C:\NODEINFO\1
C:\NODEINFO\1
Sys Op
Caller Name
05:41
Y
N
Y
25
1
03-15-26
05:41
05:41
32768
0
0
0

0
0
```

## Batch Files

Each door needs a `.bat` file in `C:\DOORS\` that sets up the
environment and launches the game. These run inside DOS.

**lord.bat** - Legend of the Red Dragon:

```bat
@ECHO OFF
C:
CD \DOORS\LORD
LORD.EXE %1 /DREW
EXITEMU
```

The `%1` parameter is the node number. `/DREW` tells LORD to use serial
I/O (remote mode). Without `/DREW`, LORD writes directly to the screen.

LORD also requires `NODE0.DAT` in its directory with at least:

```
COMPORT 1
BBSTYPE DOORSYS
```

And a `DOOR.SYS` file copied or linked into the LORD directory from
`C:\NODEINFO\<node>\DOOR.SYS`.

**bre.bat** - Barren Realms Elite:

```bat
@ECHO OFF
C:
CD \DOORS\BRE
DEL \DOORS\BRE\IN_USE*.*
DEL \DOORS\BRE\INUSE*.*
SRDOOR -f setup.1
BRE
EXITEMU
```

BRE uses SRDOOR to convert drop files. The `setup.1` file configures
which drop file format to read and where to find it.

**darkness.bat** - Darkness:

```bat
@ECHO OFF
C:
CD \DOORS\DARKNESS
COPY C:\NODEINFO\%1\DORINFO%1.DEF .
DARKNESS /N%1
EXITEMU
```

**ooii.bat** - Operation: Overkill II:

```bat
@ECHO OFF
C:
CD \DOORS\OOII
OOINFO 5 \NODEINFO\%1\ %1
MAINTOO
OOII
EXITEMU
```

OOII uses OOINFO to convert drop files. The `5` parameter selects the
DORINFO format. MAINTOO runs daily maintenance.

**simp.bat** - The Simpsons:

```bat
@ECHO OFF
C:
CD \DOORS\SIMP
COPY c:\NODEINFO\%1\DORINFO%1.DEF .
SIMPSONS %1
EXITEMU
```

## Running the Test Harness

The test harness (`tests/doortest.py`) allocates a PTY, starts dosdoor
with virtual COM1, captures output, and optionally injects keystrokes:

```bash
# Basic door test (30 second timeout)
python3 tests/doortest.py -t 30 -- "c:\\doors\\lord.bat 1"

# Send Enter key after 15 seconds (to dismiss title screen)
python3 tests/doortest.py -t 30 -k "\r" --keys-delay 15 -- "c:\\doors\\lord.bat 1"

# Test BRE
python3 tests/doortest.py -t 30 -- "c:\\doors\\bre.bat 1"

# Test without serial (screen output mode, no COM1)
python3 tests/doortest.py -t 20 --no-serial -- "c:\\doors\\darkness.bat 1"
```

Options:
- `-t SECONDS` - timeout (default: 30)
- `-c CPU` - CPU emulation mode: `fullsim`, `vm86`, `jit` (default: fullsim)
- `-k KEYS` - keystrokes to inject (supports `\r`, `\n`, `\x1b`)
- `--keys-delay SECONDS` - delay before injecting keys
- `-q` - quiet mode (suppress harness status messages)

## Running Manually

```bash
# Interactive terminal mode with virtual COM1
./build/bin/dosdoor -5 -t \
  --Flibdir /tmp/dosemu_lib \
  -F /tmp/dosemu_etc/global.conf \
  -f /tmp/dosemu_etc/dosemu.conf \
  -I "serial { com 1 virtual } cpuemu fullsim" \
  "c:\\doors\\lord.bat 1"

# With debug logging
./build/bin/dosdoor -5 -t \
  --Flibdir /tmp/dosemu_lib \
  -F /tmp/dosemu_etc/global.conf \
  -f /tmp/dosemu_etc/dosemu.conf \
  -I "serial { com 1 virtual } cpuemu fullsim" \
  -o /tmp/dosdoor_debug.log -D+a \
  "c:\\doors\\lord.bat 1"

# Plain DOS prompt (no door, no serial)
./build/bin/dosdoor -5 -t \
  --Flibdir /tmp/dosemu_lib \
  -F /tmp/dosemu_etc/global.conf \
  -f /tmp/dosemu_etc/dosemu.conf \
  -I "cpuemu fullsim"
```

Flags:
- `-5` - use 80x25 text mode
- `-t` - terminal mode (not X11/SDL)
- `--Flibdir` - path to the library directory
- `-F` - path to global.conf
- `-f` - path to dosemu.conf
- `-I` - inline configuration overrides
- `-o` - debug log output file
- `-D+a` - enable all debug categories

## Expected Results

| Door | Expected Output | Notes |
|------|----------------|-------|
| LORD | Dragon ASCII art, title screen, "Your choice, warrior?" menu | Needs /DREW flag and DOOR.SYS |
| BRE | "Barren Realms Elite v0.988", "Registered To:" | Needs SRDOOR + setup.1 |
| Darkness | Loads CONFIG.DAT, game screen with "[user]" | Uses DORINFO, may exit fast without FOSSIL |
| OOII | "Daily Maintenance", "Maintenance Complete" | Needs OOINFO conversion, uses COM1 |
| Simpsons | "The Simpsons v1.2" banner, game prompt | Uses DORINFO, DoorDriver framework |

## Troubleshooting

**No output (only ~11 bytes):** When `serial { com 1 virtual }` is
active, screen output (INT 10h) is disabled. All I/O goes through the
serial port. If the door game doesn't use serial/FOSSIL, you won't see
output. Try running without serial virtual to see screen output.

**Door exits immediately:** Check that drop files (DOOR.SYS,
DORINFO1.DEF) exist in `C:\NODEINFO\<node>\` and contain valid data.
Many doors validate the drop file on startup and exit if it's missing
or malformed.

**LORD shows "Range check error":** Use the full 52-line DOOR.SYS
format (not a short version). The range check error is caused by LORD
parsing fields at wrong positions.

**"Unable to open Comport":** The door needs `serial { com 1 virtual }`
in the dosdoor configuration. Some doors (OOII, Darkness) open COM1
directly and will fail without it.

**CPU emulation modes:**
- `fullsim` - pure software interpreter, works on all platforms
  (x86_64, ARM64), slower but reliable
- `vm86` - uses Linux vm86() syscall, only on x86_64 Linux
- `jit` / `full` - JIT compilation to native x86, only on x86_64,
  fastest but may have stability issues
