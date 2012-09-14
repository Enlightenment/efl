ACLOCAL_AMFLAGS = -I m4

if !BUILD_DOCS_ONLY
SUBDIRS = src doc data

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

bin_SCRIPTS = 

EXTRA_DIST = \
AUTHORS \
COPYING \
COPYING_ARTS \
autogen.sh \
ephysics.pc.in \
README

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ephysics.pc
endif

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc