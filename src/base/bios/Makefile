top_builddir=../../..
include $(top_builddir)/Makefile.conf

all: lib

ifeq ($(OS),darwin)
CFILES=int10.c int16.c hlt.c setup.c bios_data.c
else
CFILES=int10.c int16.c hlt.c setup.c
OBJS=$(CFILES:.c=.o) bios.o

bios.o: bios.S
	$(CC) $(CPPFLAGS) -c -Wa,-a -traditional $< > $*.s.out
endif

include $(REALTOPDIR)/src/Makefile.common

clean::
	rm -f *.s.out
