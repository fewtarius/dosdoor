#!/bin/bash
#
# test-homebrew.sh - simulate a Homebrew-style build and install
#
# Builds dosdoor with PREFIX=/opt/homebrew (the Homebrew default on
# Apple Silicon), installs to a staging directory, and verifies the
# layout matches what the Homebrew formula would produce.
#
# Usage:
#   tests/test-homebrew.sh [prefix]
#
# prefix defaults to /opt/homebrew on ARM macOS, /usr/local otherwise.
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TOP="$(cd "$SCRIPT_DIR/.." && pwd)"
STAGING="/tmp/dosdoor-brew-test-$$"
ERRORS=0

pass() { printf "  \033[32m\033[0m %s\n" "$1"; }
fail() { printf "  \033[31m\033[0m %s\n" "$1"; ERRORS=$((ERRORS + 1)); }
warn() { printf "  \033[33m!\033[0m %s\n" "$1"; }

cleanup() {
    rm -r "$STAGING" 2>/dev/null || true
}
trap cleanup EXIT

# Detect prefix
if [ -n "${1:-}" ]; then
    PREFIX="$1"
else
    case "$(uname -s)-$(uname -m)" in
        Darwin-arm64)  PREFIX="/opt/homebrew" ;;
        *)             PREFIX="/usr/local" ;;
    esac
fi

cd "$TOP"

echo "=== dosdoor Homebrew simulation test ==="
echo "Simulating: PREFIX=$PREFIX"
echo ""

# --- Build with PREFIX ---
echo "Build:"
export PREFIX
if ./build.sh >/dev/null 2>&1; then
    pass "build.sh with PREFIX=$PREFIX"
else
    fail "build.sh failed"
    echo "=== FATAL ==="
    exit 1
fi
echo ""

# --- Check compiled-in paths ---
echo "Compiled-in paths (confpath.h):"
if [ -f src/include/confpath.h ]; then
    while IFS= read -r line; do
        case "$line" in
            *ALTERNATE_ETC*)
                val="$(echo "$line" | sed 's/.*"\(.*\)".*/\1/')"
                if echo "$val" | grep -q "^$PREFIX"; then
                    pass "ALTERNATE_ETC = $val"
                else
                    fail "ALTERNATE_ETC = $val (expected $PREFIX/...)"
                fi
                ;;
            *DOSEMULIB_DEFAULT*)
                val="$(echo "$line" | sed 's/.*"\(.*\)".*/\1/')"
                if [ "$val" = "$PREFIX/share/dosdoor" ]; then
                    pass "DOSEMULIB_DEFAULT = $val"
                else
                    fail "DOSEMULIB_DEFAULT = $val (expected $PREFIX/share/dosdoor)"
                fi
                ;;
            *DOSEMUHDIMAGE_DEFAULT*)
                val="$(echo "$line" | sed 's/.*"\(.*\)".*/\1/')"
                if [ "$val" = "$PREFIX/share/dosdoor" ]; then
                    pass "DOSEMUHDIMAGE_DEFAULT = $val"
                else
                    fail "DOSEMUHDIMAGE_DEFAULT = $val (expected $PREFIX/share/dosdoor)"
                fi
                ;;
        esac
    done < src/include/confpath.h
else
    fail "confpath.h not generated"
fi
echo ""

# --- Install to staging ---
echo "Staged install:"
mkdir -p "$STAGING"
if make install DESTDIR="$STAGING" >/dev/null 2>&1; then
    pass "make install DESTDIR=$STAGING"
else
    fail "make install"
fi
echo ""

# --- Verify layout ---
echo "Layout verification:"
if "$SCRIPT_DIR/verify-install.sh" "$STAGING$PREFIX" >/dev/null 2>&1; then
    pass "verify-install passed for staged install"
else
    fail "verify-install failed"
    "$SCRIPT_DIR/verify-install.sh" "$STAGING$PREFIX" 2>&1 | grep '' | head -15
fi
echo ""

# --- Homebrew-specific checks ---
echo "Homebrew-specific:"

# Check that drive_z has all FreeDOS utilities (not just built commands)
DZ="$STAGING$PREFIX/share/dosdoor/drive_z"
if [ -f "$DZ/dosemu/emufs.sys" ] && [ -f "$DZ/dosemu/fossil.com" ]; then
    pass "drive_z has FreeDOS utilities (emufs.sys, fossil.com)"
else
    fail "drive_z missing FreeDOS utilities"
fi

if [ -f "$DZ/command.com" ]; then
    pass "drive_z has command.com"
else
    fail "drive_z missing command.com"
fi

# Check drives/c exists
DC="$STAGING$PREFIX/share/dosdoor/drives/c"
if [ -d "$DC" ]; then
    pass "drives/c exists (hdimage default)"
else
    fail "drives/c missing"
fi

if [ -d "$DC/tmp" ]; then
    pass "drives/c/tmp exists (TEMP dir)"
else
    fail "drives/c/tmp missing"
fi

# Check config files
CONF="$STAGING$PREFIX/etc/dosdoor"
if [ -f "$CONF/global.conf" ] && [ -f "$CONF/dosemu.conf" ]; then
    pass "config files installed"
else
    fail "config files missing"
fi

# Check no stale drives/* references in compiled binary
BINARY="$STAGING$PREFIX/bin/dosdoor"
if [ -f "$BINARY" ]; then
    if strings "$BINARY" | grep -q 'drives/\*'; then
        fail "binary still contains 'drives/*' string"
    else
        pass "binary has no 'drives/*' references"
    fi
fi
echo ""

# --- Summary ---
echo "=== Results ==="
if [ "$ERRORS" -eq 0 ]; then
    printf "\033[32mAll Homebrew simulation tests passed.\033[0m\n"
else
    printf "\033[31m%d test(s) failed.\033[0m\n" "$ERRORS"
fi

exit "$ERRORS"
