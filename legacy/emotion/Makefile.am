## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.sub \
configure \
depcomp \
install-sh \
ltconfig \
ltmain.sh \
missing \
mkinstalldirs \
stamp-h.in \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2

EXTRA_DIST = \
README \
AUTHORS \
COPYING \
autogen.sh \
emotion.spec \
emotion.spec.in \
emotion.pc.in \
m4/ac_attribute.m4 \
m4/efl_binary.m4 \
m4/efl_doxygen.m4 \
m4/efl_edje_external.m4 \
m4/efl_examples.m4 \
m4/efl_shm_open.m4 \
m4/emotion_check.m4 \
m4/gst-element-check.m4


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emotion.pc

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc
