ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES =	Makefile.in aclocal.m4 config.guess config.h.in* \
			config.log config.sub configure depcomp \
			install-sh ltmain.sh missing compile eeze*doc*tar* \
			m4/l*
			

pkgconfigdir = $(libdir)/pkgconfig

EEZE_PC = eeze.pc
pkgconfig_DATA = $(EEZE_PC)

.PHONY: doc

doc: all
	@echo "entering doc/"
	$(MAKE) -C doc doc
