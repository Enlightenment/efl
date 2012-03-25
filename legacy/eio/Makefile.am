ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
ABOUT-NLS \
Makefile.in \
aclocal.m4 \
compile \
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
m4/ltversion.m4

EXTRA_DIST = \
AUTHORS \
COPYING \
README \
autogen.sh \
eio.spec \
eio.pc.in \
m4/ac_attribute.m4 \
m4/efl_doxygen.m4 \
m4/efl_path_max.m4 \
m4/efl_threads.m4 \
m4/eio_check_options.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eio.pc

.PHONY: doc coverage benchmark

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc
