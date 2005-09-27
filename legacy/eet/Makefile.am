## Process this file with automake to produce Makefile.in

if BUILD_DOCS
SUBDIRS_DOCS = doc
else
SUBDIRS_DOCS =
endif

SUBDIRS = src $(SUBDIRS_DOCS)

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in build-stamp configure-stamp depcomp \
		       eet_docs.tar.gz eet.c \
		       README \
		       eet.pc \
		       eet.spec \
		       eet.oe eet-native.oe \
		       eet.bb eetXnative.bb \
		       debian/changelog

bin_SCRIPTS = eet-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN eet.c.in gendoc \
             Doxyfile \
	     README.in README \
	     eet.pc.in \
	     eet.spec.in eet.spec \
	     eet.oe.in eet.oe eet-native.oe.in eet-native.oe \
	     eet.bb.in eet.bb eetXnative.bb.in eetXnative.bb \
	     debian/changelog.in debian/changelog \
	     debian/control \
	     debian/copyright \
	     debian/eet-config.1 \
	     debian/eet.1 \
	     debian/libeet0-dev.files \
	     debian/libeet0.files \
	     debian/rules

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eet.pc

