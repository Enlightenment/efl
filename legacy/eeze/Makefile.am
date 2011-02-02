ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
compile \
config.guess \
config.h.in \
config.h.in~ \
config.sub \
configure \
depcomp \
install-sh \
ltconfig \
ltmain.sh \
missing \
eeze*doc*tar* \
eeze.pc \
eeze.spec \
m4/l*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eeze.pc

EXTRA_DIST = \
AUTHORS \
COPYING \
README \
$(pkgconfig_DATA) \
autogen.sh \
eeze.pc.in \
eeze.spec.in \
eeze.spec \
m4/efl_doxygen.m4

.PHONY: doc

# Documentation

doc: all
	@echo "entering doc/"
	$(MAKE) -C doc doc
