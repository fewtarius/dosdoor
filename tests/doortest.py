#!/usr/bin/env python3
"""
tests/doortest.py - BBS door game test harness for dosdoor

Simulates how a BBS (like PhotonBBS) calls a door game:
  - allocates a PTY so dosdoor gets a real terminal for COM1 virtual
  - captures ANSI output from the door game
  - can inject keystrokes from stdin or a key file
  - supports timeout

Usage:
    python3 tests/doortest.py [options] [-- door_command]

Examples:
    python3 tests/doortest.py -- "c:\\doors\\lord.bat 1"
    python3 tests/doortest.py -c vm86 -t 60 -- "c:\\doors\\lord.bat 1"
    echo "Q" | python3 tests/doortest.py -t 10 -- "c:\\doors\\lord.bat 1"
"""

import argparse
import errno
import os
import pty
import select
import signal
import subprocess
import sys
import time


def find_binary(candidates):
    for c in candidates:
        if os.path.isfile(c) and os.access(c, os.X_OK):
            return c
    return None


def find_dir(candidates):
    for c in candidates:
        if os.path.isdir(c):
            return c
    return None


def log(msg, quiet=False):
    if not quiet:
        print(f"[doortest] {msg}", file=sys.stderr)


def main():
    parser = argparse.ArgumentParser(description="BBS door game test harness")
    parser.add_argument("-t", "--timeout", type=int, default=30,
                        help="timeout in seconds (default: 30)")
    parser.add_argument("-c", "--cpu", default="auto",
                        choices=["auto", "full", "fullsim", "vm86", "off"],
                        help="CPU emulation mode (default: auto - fullsim on ARM, full on x86)")
    parser.add_argument("-n", "--node", type=int, default=1,
                        help="node number (default: 1)")
    parser.add_argument("-d", "--debug", action="store_true",
                        help="enable dosdoor debug logging")
    parser.add_argument("-q", "--quiet", action="store_true",
                        help="suppress harness messages")
    parser.add_argument("-k", "--keys", type=str, default=None,
                        help="keystrokes to send after startup (e.g. 'Q\\r')")
    parser.add_argument("--keys-delay", type=float, default=3.0,
                        help="seconds to wait before sending keys (default: 3)")
    parser.add_argument("-b", "--binary", type=str, default=None,
                        help="path to dosdoor binary")
    parser.add_argument("-l", "--libdir", type=str, default=None,
                        help="path to dosemu lib directory")
    parser.add_argument("--confdir", type=str, default=None,
                        help="path to dosemu config directory")
    parser.add_argument("--raw", action="store_true",
                        help="output raw bytes (don't filter)")
    parser.add_argument("door_command", nargs="?", default=None,
                        help="DOS command to run (e.g. c:\\\\doors\\\\lord.bat 1)")

    args = parser.parse_args()

    # Find dosdoor binary
    if args.binary:
        dosdoor = args.binary
    else:
        dosdoor = find_binary([
            "./build/bin/dosdoor",
            "./1.4.0.1/bin/dosdoor",
            "../dosemu/build/bin/dosdoor",
        ])
    if not dosdoor:
        print("ERROR: Cannot find dosdoor binary", file=sys.stderr)
        sys.exit(1)

    # Find lib directory
    if args.libdir:
        libdir = args.libdir
    else:
        libdir = find_dir([
            "/tmp/dosemu_lib",
            os.path.expanduser("~/.dosemu"),
            "/opt/photonbbs/.dosemu",
        ])
    if not libdir:
        print("ERROR: Cannot find dosemu lib directory", file=sys.stderr)
        sys.exit(1)

    # Find config directory
    if args.confdir:
        confdir = args.confdir
    else:
        confdir = find_dir(["/tmp/dosemu_etc"])
    if not confdir or not os.path.isfile(os.path.join(confdir, "global.conf")):
        print("ERROR: Cannot find dosemu config directory", file=sys.stderr)
        sys.exit(1)

    # Auto-detect CPU mode based on platform
    import platform
    cpu_mode = args.cpu
    if cpu_mode == "auto":
        machine = platform.machine().lower()
        if machine in ("arm64", "aarch64"):
            cpu_mode = "fullsim"
        else:
            cpu_mode = "full"
    log(f"binary:  {dosdoor}", args.quiet)
    log(f"libdir:  {libdir}", args.quiet)
    log(f"confdir: {confdir}", args.quiet)
    log(f"cpu:     {cpu_mode}", args.quiet)
    log(f"timeout: {args.timeout}s", args.quiet)

    # Build command line
    cmd = [dosdoor, "-5", "-t",
           "--Flibdir", libdir,
           "-F", os.path.join(confdir, "global.conf"),
           "-f", os.path.join(confdir, "dosemu.conf"),
           "-I", f"serial {{ com 1 virtual }} cpuemu {cpu_mode}"]

    if args.debug:
        cmd.extend(["-o", "/tmp/doortest_debug.log", "-D+d"])
        log("debug log: /tmp/doortest_debug.log", args.quiet)

    if args.door_command:
        cmd.append(args.door_command)
        log(f"door: {args.door_command}", args.quiet)

    # Create a PTY pair
    master_fd, slave_fd = pty.openpty()

    # Set terminal size on the slave
    import struct
    import fcntl
    import termios
    winsize = struct.pack("HHHH", 25, 80, 0, 0)
    fcntl.ioctl(slave_fd, termios.TIOCSWINSZ, winsize)

    # Set the PTY slave to raw mode so our responses aren't echoed
    # and slang can properly read them
    attrs = termios.tcgetattr(slave_fd)
    # Turn off echo and canonical mode, but keep basic signal handling
    attrs[3] &= ~(termios.ECHO | termios.ICANON | termios.ISIG |
                   termios.IEXTEN)
    # Turn off input processing
    attrs[0] &= ~(termios.ICRNL | termios.INLCR | termios.IGNCR |
                   termios.IXON | termios.IXOFF)
    # Turn off output processing
    attrs[1] &= ~termios.OPOST
    # Set minimum read to 1 byte, no timeout
    attrs[6][termios.VMIN] = 1
    attrs[6][termios.VTIME] = 0
    termios.tcsetattr(slave_fd, termios.TCSANOW, attrs)

    log("starting dosdoor with PTY...", args.quiet)

    # Fork dosdoor with the slave PTY as stdin/stdout/stderr
    env = os.environ.copy()
    env["TERM"] = "ansi"
    env["COLUMNS"] = "80"
    env["LINES"] = "25"

    proc = subprocess.Popen(
        cmd,
        stdin=slave_fd,
        stdout=slave_fd,
        stderr=slave_fd,
        env=env,
        close_fds=True,
    )

    # Close the slave fd in the parent - child has it
    os.close(slave_fd)

    # Read output from the master fd
    output = bytearray()
    pending = bytearray()  # Buffer for detecting ANSI escape sequences
    start_time = time.time()
    keys_sent = False

    def respond_to_terminal_queries(data, master_fd, quiet):
        """Detect and respond to terminal queries from slang/curses.
        
        slang sends DSR (Device Status Report) \x1b[6n to query cursor
        position. We must respond with \x1b[row;colR or slang blocks
        forever waiting for the response.
        """
        # Look for DSR query: ESC [ 6 n
        DSR = b'\x1b[6n'
        # Look for DA (Device Attributes) query: ESC [ c or ESC [ 0 c
        DA1 = b'\x1b[c'
        DA2 = b'\x1b[0c'

        if DSR in data:
            response = b'\x1b[1;1R'  # Cursor at row 1, col 1
            os.write(master_fd, response)
            log("responded to DSR query (cursor position)", quiet)

        if DA1 in data or DA2 in data:
            response = b'\x1b[?1;2c'  # VT100 with AVO
            os.write(master_fd, response)
            log("responded to DA query (device attributes)", quiet)

    try:
        while True:
            elapsed = time.time() - start_time
            remaining = args.timeout - elapsed
            if remaining <= 0:
                log(f"timeout after {args.timeout}s", args.quiet)
                break

            # Check if process is still running
            ret = proc.poll()
            if ret is not None:
                # Process exited, drain remaining output
                while True:
                    r, _, _ = select.select([master_fd], [], [], 0.1)
                    if not r:
                        break
                    try:
                        data = os.read(master_fd, 4096)
                        if not data:
                            break
                        output.extend(data)
                    except OSError:
                        break
                log(f"dosdoor exited with code {ret}", args.quiet)
                break

            # Send keystrokes after delay
            if args.keys and not keys_sent and elapsed >= args.keys_delay:
                keys_data = args.keys.encode().decode("unicode_escape").encode("latin-1")
                log(f"sending {len(keys_data)} bytes of keystrokes", args.quiet)
                os.write(master_fd, keys_data)
                keys_sent = True

            # Wait for data
            r, _, _ = select.select([master_fd], [], [], min(0.5, remaining))
            if r:
                try:
                    data = os.read(master_fd, 4096)
                    if not data:
                        break
                    output.extend(data)

                    # Respond to terminal queries from slang
                    respond_to_terminal_queries(data, master_fd, args.quiet)

                    # Write to stdout in real time
                    if args.raw:
                        sys.stdout.buffer.write(data)
                        sys.stdout.buffer.flush()
                except OSError as e:
                    if e.errno == errno.EIO:
                        # PTY closed
                        break
                    raise

    except KeyboardInterrupt:
        log("interrupted", args.quiet)
    finally:
        # Clean up
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
        os.close(master_fd)

    # Output results
    if not args.raw:
        # Decode output, replacing non-printable bytes
        try:
            text = output.decode("cp437", errors="replace")
        except Exception:
            text = output.decode("latin-1", errors="replace")

        # Print it - ANSI codes will render in a terminal
        sys.stdout.write(text)
        sys.stdout.flush()

    log(f"captured {len(output)} bytes of output", args.quiet)

    # Save raw output for analysis
    outpath = "/tmp/doortest_output.bin"
    with open(outpath, "wb") as f:
        f.write(output)
    log(f"raw output saved to {outpath}", args.quiet)


if __name__ == "__main__":
    main()
