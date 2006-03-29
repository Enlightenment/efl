SUBDIRS = data src proj

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in build-stamp configure-stamp depcomp \
		       evas_docs.tar.gz evas.c \
		       README \
		       evas.pc \
		       evas.spec \
		       evas.oe evas-native.oe \
		       evas.bb evasXnative.bb \
		       debian/changelog

bin_SCRIPTS = evas-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN evas.c.in gendoc \
             Doxyfile \
	     README.in README \
	     evas.pc.in \
	     evas.spec.in evas.spec \
	     evas.oe.in evas.oe evas-native.oe.in evas-native.oe \
	     evas.bb.in evas.bb evasXnative.bb.in evasXnative.bb \
             debian/changelog \
	     debian/changelog.in \
	     debian/control \
	     debian/copyright \
	     debian/evas0-bin.install \
	     debian/libevas0-dev.install \
	     debian/libevas0.install \
	     debian/rules

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evas.pc
