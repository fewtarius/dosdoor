#!/bin/bash
#
# test-build.sh - validate dosdoor builds correctly
#
# Tests: clean build, binary output, version check, make install
# to staging directory, and verify-install against the staged tree.
#
# Usage:
#   tests/test-build.sh [--skip-clean]
#
# --skip-clean: skip 'make clean' and rebuild (faster iteration)
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TOP="$(cd "$SCRIPT_DIR/.." && pwd)"
STAGING="/tmp/dosdoor-test-$$"
ERRORS=0

pass() { printf "  \033[32m✓\033[0m %s\n" "$1"; }
fail() { printf "  \033[31m✗\033[0m %s\n" "$1"; ERRORS=$((ERRORS + 1)); }

cleanup() {
    rm -r "$STAGING" 2>/dev/null || true
}
trap cleanup EXIT

SKIP_CLEAN=0
for arg in "$@"; do
    case "$arg" in
        --skip-clean) SKIP_CLEAN=1 ;;
    esac
done

cd "$TOP"

echo "=== dosdoor build test ==="
echo ""

# --- Clean build ---
if [ "$SKIP_CLEAN" = 0 ]; then
    echo "Clean build:"
    if make clean >/dev/null 2>&1; then
        pass "make clean"
    else
        fail "make clean"
    fi

    if ./build.sh >/dev/null 2>&1; then
        pass "build.sh"
    else
        fail "build.sh (see build output for details)"
    fi
else
    echo "Build (incremental):"
    if make >/dev/null 2>&1; then
        pass "make"
    else
        fail "make"
    fi
fi
echo ""

# --- Binary checks ---
echo "Binary:"
BINARY="build/bin/dosdoor"
if [ -f "$BINARY" ]; then
    pass "binary exists ($BINARY)"
else
    fail "binary missing ($BINARY)"
    echo ""
    echo "=== FATAL: no binary, cannot continue ==="
    exit 1
fi

if [ -x "$BINARY" ]; then
    pass "binary is executable"
else
    fail "binary is not executable"
fi

VERSION_OUT="$("$BINARY" --version 2>&1 || true)"
if echo "$VERSION_OUT" | grep -q "dosemu-dosdoor"; then
    pass "version string: $(echo "$VERSION_OUT" | head -1)"
else
    fail "unexpected version output: $VERSION_OUT"
fi

# Check binary type matches platform
case "$(uname -s)-$(uname -m)" in
    Darwin-arm64)
        if file "$BINARY" | grep -q "arm64"; then
            pass "binary is arm64 (native Apple Silicon)"
        else
            fail "binary is not arm64: $(file "$BINARY")"
        fi
        ;;
    Darwin-x86_64)
        if file "$BINARY" | grep -q "x86_64"; then
            pass "binary is x86_64"
        else
            fail "binary architecture mismatch: $(file "$BINARY")"
        fi
        ;;
    Linux-x86_64)
        if file "$BINARY" | grep -q "x86-64"; then
            pass "binary is x86-64"
        else
            fail "binary architecture mismatch: $(file "$BINARY")"
        fi
        ;;
    Linux-aarch64)
        if file "$BINARY" | grep -q "aarch64\|ARM aarch64"; then
            pass "binary is aarch64"
        else
            fail "binary architecture mismatch: $(file "$BINARY")"
        fi
        ;;
esac
echo ""

# --- Source tree verification ---
echo "Source tree:"
if "$SCRIPT_DIR/verify-install.sh" --source >/dev/null 2>&1; then
    pass "verify-install --source passed"
else
    fail "verify-install --source failed"
    echo "  Details:"
    "$SCRIPT_DIR/verify-install.sh" --source 2>&1 | grep '✗' | head -5
fi
echo ""

# --- Staged install ---
echo "Staged install (DESTDIR=$STAGING):"
mkdir -p "$STAGING"
if make install DESTDIR="$STAGING" >/dev/null 2>&1; then
    pass "make install"
else
    fail "make install"
fi

# Detect prefix from confpath.h
PREFIX=$(grep DOSEMULIB_DEFAULT src/include/confpath.h | sed 's/.*"\(.*\)\/share.*/\1/')
if [ -z "$PREFIX" ]; then
    PREFIX="/usr/local"
fi

if "$SCRIPT_DIR/verify-install.sh" "$STAGING$PREFIX" >/dev/null 2>&1; then
    pass "verify-install against staged install"
else
    fail "verify-install against staged install"
    echo "  Details:"
    "$SCRIPT_DIR/verify-install.sh" "$STAGING$PREFIX" 2>&1 | grep '✗' | head -10
fi
echo ""

# --- Summary ---
echo "=== Results ==="
if [ "$ERRORS" -eq 0 ]; then
    printf "\033[32mAll build tests passed.\033[0m\n"
else
    printf "\033[31m%d test(s) failed.\033[0m\n" "$ERRORS"
fi

exit "$ERRORS"
