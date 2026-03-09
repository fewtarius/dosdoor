#
# Makefile for dosdoor
# Based on dosemu 1.4.0
#

SHELL=/bin/bash

all: default

srcdir=.
top_builddir=.
SUBDIR:=.
-include Makefile.conf

default clean realclean install:
	@$(MAKE) -C src $@

dosbin:
	@$(MAKE) SUBDIR:=commands -C src/commands dosbin

pristine distclean mrproper:
	@$(MAKE) -C src pristine
	rm -f Makefile.conf
	rm -f core `find . -name config.cache`
	rm -f core `find . -name config.status`
	rm -f core `find . -name config.log`
	rm -f core `find . -name configure.lineno`
	rm -f src/include/config.h
	rm -f src/include/confpath.h
	rm -f src/include/plugin_*.h
	rm -f core `find . -name '*~'`
	rm -f core `find . -name '*[\.]o'`
	rm -f core `find . -name '*.d'`
	rm -f core `find . -name '*[\.]orig'`
	rm -f core `find . -name '*[\.]rej'`
	rm -f core gen*.log `find . -size 0`
	rm -rf autom4te*.cache
