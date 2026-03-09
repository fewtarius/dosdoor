top_builddir=../../..
include $(top_builddir)/Makefile.conf

CFILES=commands.c lredir.c msetenv.c \
       unix.c cmdline.c builtins.c

D:=$(BINPATH)/commands

COM = $(D)/generic.com
STUBSYMLINK = $(D)/exitemu.com $(D)/cmdline.com \
  $(D)/lredir.com $(D)/unix.com $(D)/dpmi.com

STUBL =
STUBFULL =

OBJS=$(CFILES:.c=.o)
DEPENDS=$(CFILES:.c=.d)

all: lib $(COM) stubs

stubs: $(STUBL) $(STUBM) $(STUBFULL) stublinks

stublinks: $(STUBSYMLINK)

ifdef REALTOPDIR
include $(REALTOPDIR)/src/Makefile.common
endif

$(STUBSYMLINK): $(D)/generic.com
	ln -sf $(<F) $@

ifeq ($(OS),darwin)
# Use prebuilt generic.com on macOS (x86 assembly, can't compile on ARM)
$(D)/generic.com: prebuilt/generic.com
	mkdir -p $(D)
	cp prebuilt/generic.com $@
	chmod -x $@
else
SFILES=generic.S
DEPENDS+=$(SFILES:.S=.d)

%.o: %.S
	$(CC) $(CPPFLAGS) -m32 -traditional -Wa,-a -c -o $@ $< > $*.s.out

$(D)/%.com: %.o
	ld -m elf_i386 -Ttext 0x100 -e _start16 -s -o $@.elf $<
	objcopy -O binary -j .text $@.elf $@
	rm -f $@.elf
	chmod -x $@
endif

install: all

clean::
	rm -f *~ *.o *.s *.out *.d *.com mkcomstub
