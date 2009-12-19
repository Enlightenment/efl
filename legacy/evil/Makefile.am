
ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
aclocal.m4 \
compile \
config.guess \
config.h.in \
config.sub \
configure \
evil.pc \
depcomp \
install-sh \
ltmain.sh \
Makefile.in \
missing \
mkinstalldirs \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)* \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

SUBDIRS = src doc

EXTRA_DIST = \
COPYING \
COPYING-PLAIN \
autogen.sh \
evil.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evil.pc

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc
