top_builddir=../../..
include $(top_builddir)/Makefile.conf

# Core VGA emulator files needed for text-mode terminal output.
# Hardware-specific drivers (et4000, s3, trident, etc.), console/vc,
# and ASM blobs removed - door games only need text mode.
CFILES = miscemu.c text.c render.c video.c vgaemu.c dacemu.c attremu.c seqemu.c crtcemu.c gfxemu.c remap.c vgafonts.c instremu.c

all: lib

include $(SRCPATH)/Makefile.common
