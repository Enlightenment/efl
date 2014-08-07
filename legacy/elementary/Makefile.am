SUBDIRS = src data config doc

if HAVE_PO

SUBDIRS += po

endif

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
ABOUT-NLS \
Makefile.in \
aclocal.m4 \
config.guess \
elementary_config.h.in* \
config.sub \
config.rpath \
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
m4/ltversion.m4 \
m4/gettext.m4 \
m4/iconv.m4 \
m4/lcmessage.m4 \
m4/lib-ld.m4 \
m4/lib-link.m4 \
m4/lib-prefix.m4 \
m4/nls.m4 \
m4/po.m4 \
m4/progtest.m4

if HAVE_PO

MAINTAINERCLEANFILES += \
po/boldquot.sed \
po/en@boldquot.header \
po/en@quot.header \
po/insert-header.sin \
po/Makefile.in.in \
po/Makevars.template \
po/quot.sed \
po/remove-potcdate.sin \
po/Rules-quot

endif




EXTRA_DIST = \
README \
AUTHORS \
COPYING \
COPYING.images \
COMPLIANCE \
elementary.pc.in \
elementary.spec \
elementary.spec.in \
m4/efl_binary.m4 \
m4/efl_doxygen.m4 \
config.rpath \
pkgbuild/PKGBUILD

if HAVE_FREEBSD
pkgconfigdir = $(libdir)data/pkgconfig
else
pkgconfigdir = $(libdir)/pkgconfig
endif

pkgconfig_DATA = elementary.pc

cmakeconfigdir = $(libdir)/cmake/Elementary
cmakeconfig_DATA = \
cmakeconfig/ElementaryConfig.cmake \
cmakeconfig/ElementaryConfigVersion.cmake

.PHONY: doc screenshots

doc:
	@echo "entering doc/"
	$(MAKE) -C doc doc

screenshots: all
	@echo "entering src/examples/"
	$(MAKE) -C src/examples screenshots
