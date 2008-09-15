
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
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)*

maintainer-clean-local:
	rm -rf doc/

SUBDIRS = src

EXTRA_DIST = \
COPYING \
COPYING-PLAIN \
Doxyfile \
autogen.sh \
evil.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evil.pc
