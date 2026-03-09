/* 
 * (C) Copyright 1992, ..., 2007 the "DOSEMU-Development-Team".
 *
 * for details see file COPYING.DOSEMU in the DOSEMU distribution
 */

#include "getfd.h"

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/kd.h>
#include <sys/ioctl.h>

static int is_a_console(int fd) {
    char arg = 0;
    return (ioctl(fd, KDGKBTYPE, &arg) == 0
	    && ((arg == KB_101) || (arg == KB_84)));
}

static int open_a_console(char *fnam) {
    int fd = open(fnam, O_RDONLY);
    if (fd < 0 && errno == EACCES)
      fd = open(fnam, O_WRONLY);
    if (fd < 0)
      return -1;
    if (!is_a_console(fd)) {
      close(fd);
      return -1;
    }
    return fd;
}

int getfd() {
    int fd;
    fd = open_a_console("/dev/tty");
    if (fd >= 0) return fd;
    fd = open_a_console("/dev/tty0");
    if (fd >= 0) return fd;
    fd = open_a_console("/dev/vc/0");
    if (fd >= 0) return fd;
    fd = open_a_console("/dev/console");
    if (fd >= 0) return fd;
    for (fd = 0; fd < 3; fd++)
      if (is_a_console(fd)) return fd;
    return -1;
}

#else /* !__linux__ */

int getfd() {
    return -1; /* No console keyboard on non-Linux */
}

#endif
