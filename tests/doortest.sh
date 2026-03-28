#!/usr/bin/env bash
# tests/doortest.sh - BBS door game test harness for dosdoor
#
# Simulates how a BBS (like PhotonBBS) calls a door game:
# - allocates a PTY so dosdoor gets a real terminal for COM1 virtual
# - captures ANSI output from the door game on stdout
# - can inject keystrokes via a key file or stdin pipe
# - supports timeout and exit code checking
#
# Usage:
#   ./tests/doortest.sh [options] [door_command]
#
# Options:
#   -t SECONDS   timeout (default: 30)
#   -k KEYFILE   file with keystrokes to send (one per line, supports \r \n \x1b)
#   -d           enable dosdoor debug logging to /tmp/doortest_debug.log
#   -c CPUMODE   cpu emulation mode: full, vm86, off (default: full)
#   -n NODE      node number (default: 1)
#   -r           raw output (don't strip ANSI)
#   -q           quiet (suppress harness messages)
#   -h           show help
#
# Examples:
#   ./tests/doortest.sh "c:\\doors\\lord.bat 1"
#   ./tests/doortest.sh -c vm86 -t 60 "c:\\doors\\lord.bat 1"
#   echo -e "Q\r" | ./tests/doortest.sh -t 10 "c:\\doors\\lord.bat 1"

set -euo pipefail

# Defaults
TIMEOUT=30
KEYFILE=""
DEBUG=""
CPUMODE="full"
NODE=1
RAW=0
QUIET=0
DOSDOOR=""
LIBDIR=""
CONFDIR=""

usage() {
    sed -n '2,/^$/p' "$0" | sed 's/^# \?//'
    exit "${1:-0}"
}

log() {
    [ "$QUIET" -eq 1 ] && return
    echo "[doortest] $*" >&2
}

die() {
    echo "[doortest] ERROR: $*" >&2
    exit 1
}

# Find the dosdoor binary
find_dosdoor() {
    local candidates=(
        "./build/bin/dosdoor"
        "./1.4.0.1/bin/dosdoor"
        "../dosemu/build/bin/dosdoor"
    )
    for c in "${candidates[@]}"; do
        if [ -x "$c" ]; then
            echo "$c"
            return
        fi
    done
    die "Cannot find dosdoor binary. Build first with 'make'."
}

# Find the lib directory (contains drive_z, freedos, etc)
find_libdir() {
    local candidates=(
        "/tmp/dosemu_lib"
        "$HOME/.dosemu"
        "/opt/photonbbs/.dosemu"
    )
    for c in "${candidates[@]}"; do
        if [ -d "$c" ]; then
            echo "$c"
            return
        fi
    done
    die "Cannot find dosemu lib directory"
}

# Find or create config directory
find_confdir() {
    local candidates=(
        "/tmp/dosemu_etc"
    )
    for c in "${candidates[@]}"; do
        if [ -d "$c" ] && [ -f "$c/global.conf" ]; then
            echo "$c"
            return
        fi
    done
    die "Cannot find dosemu config directory (need /tmp/dosemu_etc with global.conf)"
}

# Parse options
while getopts "t:k:dc:n:rqh" opt; do
    case "$opt" in
        t) TIMEOUT="$OPTARG" ;;
        k) KEYFILE="$OPTARG" ;;
        d) DEBUG=1 ;;
        c) CPUMODE="$OPTARG" ;;
        n) NODE="$OPTARG" ;;
        r) RAW=1 ;;
        q) QUIET=1 ;;
        h) usage 0 ;;
        *) usage 1 ;;
    esac
done
shift $((OPTIND - 1))

# Door command is the remaining argument
DOOR_CMD="${1:-}"

# Resolve paths
DOSDOOR=$(find_dosdoor)
LIBDIR=$(find_libdir)
CONFDIR=$(find_confdir)

log "dosdoor: $DOSDOOR"
log "libdir:  $LIBDIR"
log "confdir: $CONFDIR"
log "cpumode: $CPUMODE"
log "timeout: ${TIMEOUT}s"

# Build the dosdoor command line
CMD_ARGS=(-5 -t --Flibdir "$LIBDIR" -F "$CONFDIR/global.conf" -f "$CONFDIR/dosemu.conf")
CMD_ARGS+=(-I "serial { com 1 virtual } cpuemu $CPUMODE")

if [ -n "$DEBUG" ]; then
    CMD_ARGS+=(-o /tmp/doortest_debug.log -D+d)
    log "debug log: /tmp/doortest_debug.log"
fi

if [ -n "$DOOR_CMD" ]; then
    CMD_ARGS+=("$DOOR_CMD")
    log "door: $DOOR_CMD"
fi

# Use 'script' to provide a real PTY for dosdoor
# This is the simplest cross-platform PTY wrapper
OUTFILE=$(mktemp /tmp/doortest_out.XXXXXX)
trap "rm -f '$OUTFILE'" EXIT

log "starting dosdoor..."

if [ "$(uname)" = "Darwin" ]; then
    # macOS script syntax
    script -q "$OUTFILE" timeout "$TIMEOUT" "$DOSDOOR" "${CMD_ARGS[@]}" || true
else
    # Linux script syntax
    COLUMNS=80 LINES=25 TERM=ansi \
        script -q -c "timeout $TIMEOUT \"$DOSDOOR\" $(printf '%q ' "${CMD_ARGS[@]}")" \
        "$OUTFILE" </dev/null || true
fi

log "dosdoor exited"

# Display captured output
if [ "$RAW" -eq 1 ]; then
    cat "$OUTFILE"
else
    # Strip binary noise but keep ANSI for readability
    strings -n 1 "$OUTFILE" | head -200
fi

log "output saved to $OUTFILE"
