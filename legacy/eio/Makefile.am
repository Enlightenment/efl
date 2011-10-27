ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
ABOUT-NLS \
Makefile.in \
aclocal.m4 \
config.cache-env \
config.guess \
config.h.in \
config.h.in~ \
config.rpath \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
mkinstalldirs \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4 \
m4/codeset.m4 \
m4/gettext.m4 \
m4/glibc21.m4 \
m4/iconv.m4 \
m4/intdiv0.m4 \
m4/inttypes_h.m4 \
m4/inttypes.m4 \
m4/inttypes-pri.m4 \
m4/isc-posix.m4 \
m4/lcmessage.m4 \
m4/lib-ld.m4 \
m4/lib-link.m4 \
m4/lib-prefix.m4 \
m4/nls.m4 \
m4/po.m4 \
m4/progtest.m4 \
m4/stdint_h.m4 \
m4/uintmax_t.m4 \
m4/ulonglong.m4

EXTRA_DIST = \
AUTHORS \
COPYING \
autogen.sh \
eio.spec \
eio.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eio.pc

.PHONY: doc coverage benchmark

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc
