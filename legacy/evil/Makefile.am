
ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
aclocal.m4 \
config.guess \
config.h.in \
config.h.in~ \
config.sub \
configure \
depcomp \
INSTALL \
install-sh \
ltmain.sh \
Makefile.in \
missing \
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
