top_builddir=../../..
include $(top_builddir)/Makefile.conf

ifeq ($(OS),darwin)
CFILES = dpmi.c memory.c msdos.c emu-ldt.c dpmisel_data.c
else
CFILES = dpmi.c memory.c msdos.c emu-ldt.c dpmisel.o
endif

all:  lib

clean::
	-rm -f *.com

install:

include $(REALTOPDIR)/src/Makefile.common
