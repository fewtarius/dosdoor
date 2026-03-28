#!/bin/bash
#
# verify-install.sh - validate dosdoor install layout
#
# Checks that all files referenced by config.sys, autoexec.bat, and
# global.conf exist in the installed tree. Run after 'make install',
# 'brew install', or RPM install to catch path/layout problems.
#
# Usage:
#   tests/verify-install.sh [prefix]
#
# prefix defaults to /usr/local on macOS, /usr on Linux.
# For Homebrew Cellar installs: tests/verify-install.sh /opt/homebrew
# For source tree (no install):  tests/verify-install.sh --source
#

set -euo pipefail

ERRORS=0
WARNINGS=0

pass() { printf "  \033[32m✓\033[0m %s\n" "$1"; }
fail() { printf "  \033[31m✗\033[0m %s\n" "$1"; ERRORS=$((ERRORS + 1)); }
warn() { printf "  \033[33m!\033[0m %s\n" "$1"; WARNINGS=$((WARNINGS + 1)); }

check_file() {
    if [ -f "$1" ]; then
        pass "$2"
    else
        fail "$2 (missing: $1)"
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        pass "$2"
    else
        fail "$2 (missing: $1)"
    fi
}

check_link_or_file() {
    if [ -f "$1" ] || [ -L "$1" ]; then
        pass "$2"
    else
        fail "$2 (missing: $1)"
    fi
}

# Determine paths
SOURCE_MODE=0
if [ "${1:-}" = "--source" ]; then
    SOURCE_MODE=1
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
    TOP="$(cd "$SCRIPT_DIR/.." && pwd)"
    BINDIR="$TOP/build/bin"
    DATADIR="$TOP"
    CONFDIR="$TOP/etc"
    # In source mode, there's no separate drive_z - freedos/ IS the boot dir
    # and built commands live in build/commands/. The install target merges them.
    DRIVE_Z=""
    FREEDOS="$TOP/freedos"
    KEYMAPDIR="$TOP/etc/keymap"
    BUILT_COMMANDS="$TOP/build/commands"
elif [ -n "${1:-}" ]; then
    PREFIX="$1"
    BINDIR="$PREFIX/bin"
    DATADIR="$PREFIX/share/dosdoor"
    CONFDIR="$PREFIX/etc/dosdoor"
    DRIVE_Z="$DATADIR/drive_z"
    FREEDOS="$DATADIR/freedos"
    KEYMAPDIR="$DATADIR/keymap"
    BUILT_COMMANDS=""
else
    case "$(uname -s)" in
        Darwin) PREFIX="/usr/local" ;;
        *)      PREFIX="/usr" ;;
    esac
    BINDIR="$PREFIX/bin"
    DATADIR="$PREFIX/share/dosdoor"
    CONFDIR="$PREFIX/etc/dosdoor"
    DRIVE_Z="$DATADIR/drive_z"
    FREEDOS="$DATADIR/freedos"
    KEYMAPDIR="$DATADIR/keymap"
    BUILT_COMMANDS=""
fi

echo "=== dosdoor install verification ==="
echo ""
echo "Checking paths:"
echo "  bin:     $BINDIR"
echo "  data:    $DATADIR"
echo "  config:  $CONFDIR"
echo "  drive_z: $DRIVE_Z"
echo ""

# --- Binary ---
echo "Binary:"
check_file "$BINDIR/dosdoor" "dosdoor binary"
echo ""

# --- Configuration ---
echo "Configuration files:"
check_file "$CONFDIR/global.conf" "global.conf"
check_file "$CONFDIR/dosemu.conf" "dosemu.conf"
echo ""

# --- Z: drive (runtime system drive) ---
# These are referenced by config.sys and autoexec.bat:
#   device=z:\dosemu\emufs.sys
#   shellhigh=z:\command.com
#   path z:\dosemu
#   z:\dosemu\fossil.com
if [ -n "$DRIVE_Z" ]; then
echo "Z: drive layout (drive_z/):"
check_file "$DRIVE_Z/command.com" "command.com (shellhigh=z:\\command.com)"
check_dir  "$DRIVE_Z/dosemu" "dosemu/ directory (path z:\\dosemu)"

echo ""
echo "Z: drive - config.sys references:"
check_file "$DRIVE_Z/dosemu/emufs.sys" "emufs.sys (device=z:\\dosemu\\emufs.sys)"

echo ""
echo "Z: drive - autoexec.bat references:"
check_file "$DRIVE_Z/dosemu/fossil.com" "fossil.com (z:\\dosemu\\fossil.com)"

echo ""
echo "Z: drive - built commands:"
check_file "$DRIVE_Z/dosemu/exitemu.com" "exitemu.com"
check_file "$DRIVE_Z/dosemu/generic.com" "generic.com"
check_file "$DRIVE_Z/dosemu/lredir.com" "lredir.com"
check_file "$DRIVE_Z/dosemu/unix.com" "unix.com"
check_link_or_file "$DRIVE_Z/dosemu/dpmi.com" "dpmi.com (-> generic.com)"
check_link_or_file "$DRIVE_Z/dosemu/cmdline.com" "cmdline.com (-> generic.com)"

echo ""
echo "Z: drive - FreeDOS utilities:"
check_file "$DRIVE_Z/dosemu/ems.sys" "ems.sys"
check_file "$DRIVE_Z/dosemu/system.com" "system.com"

echo ""
echo "C: drive (hdimage default):"
check_dir  "$DATADIR/drives/c" "drives/c directory"
check_dir  "$DATADIR/drives/c/tmp" "drives/c/tmp directory"
fi

# --- FreeDOS hdimage boot directory ---
echo ""
echo "FreeDOS boot directory (freedos/):"
check_file "$FREEDOS/kernel.sys" "kernel.sys"
check_file "$FREEDOS/command.com" "command.com"
check_file "$FREEDOS/config.sys" "config.sys"
check_file "$FREEDOS/autoexec.bat" "autoexec.bat"
check_dir  "$FREEDOS/dosemu" "dosemu/"
check_file "$FREEDOS/dosemu/emufs.sys" "emufs.sys"
check_file "$FREEDOS/dosemu/ems.sys" "ems.sys"
check_file "$FREEDOS/dosemu/fossil.com" "fossil.com"

# --- Keymap ---
echo ""
echo "Keymap:"
check_dir  "$KEYMAPDIR" "keymap directory"
check_file "$KEYMAPDIR/us" "us keymap"

# --- Source mode: check built commands match source ---
if [ "$SOURCE_MODE" = 1 ] && [ -n "$BUILT_COMMANDS" ]; then
    echo ""
    echo "Build output (source mode):"
    check_file "$BUILT_COMMANDS/exitemu.com" "built exitemu.com"
    check_file "$BUILT_COMMANDS/generic.com" "built generic.com"
    check_file "$BUILT_COMMANDS/lredir.com" "built lredir.com"
    check_file "$BUILT_COMMANDS/unix.com" "built unix.com"

    echo ""
    echo "Source freedos/ has all files needed for install:"
    # These must exist in source tree for install targets to copy
    check_file "$TOP/freedos/command.com" "freedos/command.com"
    check_file "$TOP/freedos/kernel.sys" "freedos/kernel.sys"
    check_file "$TOP/freedos/dosemu/emufs.sys" "freedos/dosemu/emufs.sys"
    check_file "$TOP/freedos/dosemu/ems.sys" "freedos/dosemu/ems.sys"
    check_file "$TOP/freedos/dosemu/fossil.com" "freedos/dosemu/fossil.com"
    check_file "$TOP/freedos/dosemu/system.com" "freedos/dosemu/system.com"
fi

# --- Cross-reference: config.sys -> drive_z ---
if [ "$SOURCE_MODE" != 1 ] && [ -n "$DRIVE_Z" ]; then
    echo ""
    echo "Cross-reference: freedos/config.sys -> drive_z:"
    # Parse config.sys for device= and shell= lines, verify targets exist
    # The file uses DOS-style double backslashes: z:\\dosemu\\emufs.sys
    if [ -f "$FREEDOS/config.sys" ]; then
        while IFS= read -r line; do
            # Normalize: strip CR, convert backslashes to forward slashes, collapse //
            norm="$(printf '%s' "$line" | tr -d '\r' | tr '\\' '/' | sed 's|//|/|g')"
            case "$norm" in
                device=z:/dosemu/*|DEVICE=z:/dosemu/*)
                    fname="${norm##*/}"
                    check_file "$DRIVE_Z/dosemu/$fname" "config.sys device -> $fname"
                    ;;
                shellhigh=z:/*|SHELLHIGH=z:/*)
                    # Extract path after z:/, strip args
                    fpath="$(printf '%s' "$norm" | sed 's/.*z:\///' | sed 's/ .*//')"
                    check_file "$DRIVE_Z/$fpath" "config.sys shell -> $fpath"
                    ;;
            esac
        done < "$FREEDOS/config.sys"
    fi
fi

# --- Summary ---
echo ""
echo "=== Results ==="
if [ "$ERRORS" -eq 0 ] && [ "$WARNINGS" -eq 0 ]; then
    printf "\033[32mAll checks passed.\033[0m\n"
elif [ "$ERRORS" -eq 0 ]; then
    printf "\033[33m%d warning(s), 0 errors.\033[0m\n" "$WARNINGS"
else
    printf "\033[31m%d error(s), %d warning(s).\033[0m\n" "$ERRORS" "$WARNINGS"
fi

exit "$ERRORS"
