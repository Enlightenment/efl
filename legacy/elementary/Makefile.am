SUBDIRS = src data config doc

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
elementary_config.h.in \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4


EXTRA_DIST = \
README \
AUTHORS \
COPYING \
autogen.sh \
elementary.pc.in \
elementary.spec \
elementary.spec.in \
m4/ac_attribute.m4 \
m4/efl_binary.m4 \
m4/efl_doxygen.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = elementary.pc

.PHONY: doc

doc:
	@echo "entering doc/"
	$(MAKE) -C doc doc
