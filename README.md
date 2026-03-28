# dosdoor

A stripped-down, cross-platform DOS emulator purpose-built for running BBS door
games. Derived from [dosemu 1.4.0](http://www.dosemu.org/) with sound, graphics,
networking, and other subsystems removed - leaving only what door games need.

## Supported platforms

| Platform | CPU mode | Status |
|----------|----------|--------|
| Linux x86_64 | Full interpreter (fullsim) | Tested |
| Linux x86_64 | JIT compiler | Tested |
| macOS ARM64 (Apple Silicon) | Full interpreter (fullsim) | Tested |
| macOS x86_64 | Full interpreter (fullsim) | Tested |

## Tested door games

| Game | Version | Status |
|------|---------|--------|
| Legend of the Red Dragon (LORD) | 3.50 | Full serial I/O, ANSI art, gameplay |
| Barren Realms Elite (BRE) | 0.988 | Full game flow, registration, gameplay |
| Operation: Overkill II (OOII) | - | Daily maintenance, gameplay |
| The Simpsons | 1.2 | Title screen, serial I/O, gameplay |
| Darkness | - | Serial detection, clean launch |

## Installing

### Homebrew (macOS)

```bash
brew tap fewtarius/tap
brew install dosdoor
```

### RPM (Fedora/RHEL)

Download the RPM from the [latest release](https://github.com/fewtarius/dosdoor/releases)
or build from the spec file:

```bash
rpmbuild -bb dosdoor.spec
```

## Building

### Requirements

- C compiler (gcc or clang)
- GNU make
- flex and bison
- slang library (libslang2-dev / slang-devel)

### Quick build

```bash
./build.sh
```

### Manual build

```bash
# Linux
./configure && make

# macOS
cp Makefile.conf.darwin Makefile.conf && make
```

The binary is placed at `build/bin/dosdoor`.

## Usage

### Door game launch

```bash
# PhotonBBS-compatible invocation
dosdoor -t -quiet -I "serial { com 1 virtual }" "c:\\doors\\lord.bat 1"
```

### Interactive DOS prompt

```bash
dosdoor -t -quiet
```

### Common options

| Option | Description |
|--------|-------------|
| `-t` | Terminal mode (text-only, no GUI) |
| `-quiet` | Suppress startup banner |
| `-I "..."` | Inline configuration |
| `-o <file>` | Debug log output |
| `-5` | Emulate 586 (Pentium) CPU |
| `-F <dir>` | Override lib directory |
| `-f <file>` | User configuration file |

### CPU emulation modes

The `-I` option accepts `cpuemu` settings:

| Mode | Flag | Description |
|------|------|-------------|
| Auto | (default) | JIT on x86_64, fullsim elsewhere |
| Full interpreter | `cpuemu fullsim` | Pure software emulation, all platforms |
| JIT compiler | `cpuemu full` | Native code generation, x86_64 only |

## Directory layout

dosdoor uses a standard dosemu-compatible directory structure:

```
~/.dosemu/
├── drives/
│   └── c/              Host filesystem mapped as DOS C:\
│       ├── doors/      Door game directories
│       │   ├── lord/   LORD files + NODE*.DAT
│       │   ├── bre/    BRE files + setup configs
│       │   └── ...
│       ├── nodeinfo/   Per-node drop files
│       │   └── 1/
│       │       └── DORINFO1.DEF
│       └── tmp/
└── freedos/            FreeDOS kernel + utilities (bundled)
```

## Door game setup

Each door game needs:
1. Game files in `C:\DOORS\<game>\`
2. A batch file to launch the game
3. Drop files (DORINFO1.DEF, DOOR.SYS) in a known location

### Example batch file (lord.bat)

```batch
@ECHO OFF
C:
CD \DOORS\LORD
LORD %1 /DREW
EXITEMU
```

### Drop file formats

dosdoor works with standard BBS drop file formats:
- **DORINFO1.DEF** - Most games (Darkness, Simpsons, OOII)
- **DOOR.SYS** - 52-line format (LORD requires this)
- **SRDOOR** - BRE uses its own setup utility

## PhotonBBS integration

dosdoor is a drop-in replacement for dosemu in PhotonBBS doorexec scripts:

```bash
# Replace 'dosemu' with 'dosdoor' in existing scripts
dosdoor -t -quiet -I "serial { com 1 virtual }" "c:\\doors\\lord.bat $2"
```

The `DOSEMU_HOGTHRESHOLD` environment variable is accepted for backward
compatibility (dosdoor has built-in idle detection).

See [INTEGRATION.md](INTEGRATION.md) for detailed migration steps.

## Architecture

```
dosdoor
├── CPU emulation (src/emu-i386/)
│   ├── simx86 interpreter    All platforms (fullsim)
│   └── simx86 JIT compiler   x86_64 only (full)
├── Serial I/O (src/base/serial/)
│   ├── Virtual COM ports     stdin/stdout mapping
│   ├── UART emulation        16550A register-level
│   ├── INT 14h               BIOS serial services
│   └── FOSSIL driver         TSR-based serial API
├── Terminal (src/plugin/term/)
│   └── ANSI/VT100 output     via slang library
├── Filesystem (src/dosext/mfs/)
│   └── Host dir mapping      C:\ = ~/.dosemu/drives/c/
├── DPMI (src/dosext/dpmi/)
│   └── Protected mode        For doors that need it
├── BIOS (src/base/bios/)
│   ├── INT 10h               Text-mode video
│   ├── INT 16h               Keyboard input
│   └── INT 21h               DOS services
└── Platform (src/arch/)
    ├── linux/                 Signal handling, memory mapping
    └── darwin/                macOS signal handling, mapping
```

### Critical I/O path

```
User terminal <-> stdin/stdout <-> virtual COM1 <-> DOS serial API <-> Door game
```

## What was removed from dosemu

dosdoor strips everything door games don't need:

- Sound (SB16, MIDI, PC speaker, ALSA, OSS)
- Graphics (VGA, SVGA, VESA, X11, SDL)
- Mouse support
- Networking (IPX, TCP/IP, packet driver)
- SCSI and CDROM
- DMA controller
- Joystick
- GPM mouse integration
- Non-CP437 character sets
- Interactive debugger

## License

GPLv2, inherited from dosemu. See [COPYING](COPYING) for the full license text
and [COPYING.DOSEMU](COPYING.DOSEMU) for the original dosemu copyright notice.

## Credits

dosdoor is built on the work of the dosemu development team. The original
dosemu 1.4.0 was released in 2007 and remains one of the most complete DOS
emulation implementations for Linux.
