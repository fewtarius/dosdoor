top_builddir=../../..
include $(top_builddir)/Makefile.conf

ifeq ($(OS),darwin)
CFILES=hma.c ioctl.c disks.c utilities.c dos2linux.c dump.c lowmem.c \
	priv.c fatfs.c smalloc.c dlmalloc.c fatfs_boot_data.c stubs.c
else
CFILES=hma.c ioctl.c disks.c utilities.c dos2linux.c dump.c lowmem.c \
	priv.c fatfs.c smalloc.c dlmalloc.c fatfs_boot.o stubs.c
endif

include $(REALTOPDIR)/src/Makefile.common

clean::
	rm -f *.out
