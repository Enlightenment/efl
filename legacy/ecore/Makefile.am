## Process this file with automake to produce Makefile.in

SUBDIRS = data src examples

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in  build-stamp configure-stamp depcomp \
		       ecore_docs.tar.gz ecore.c \
		       README \
		       ecore.pc \
		       ecore.spec \
		       ecore.oe ecore-native.oe \
		       ecore.bb ecoreXnative.bb \
		       debian/changelog

bin_SCRIPTS = ecore-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN ecore.c.in gendoc ecore.supp ecore.m4 \
             Doxyfile \
	     ecore.pc.in \
	     ecore.pc.in \
	     ecore.spec.in ecore.spec \
	     ecore.oe.in ecore.oe ecore-native.oe.in ecore-native.oe \
	     ecore.bb.in ecore.bb ecoreXnative.bb.in ecoreXnative.bb \
	     debian/changelog \
	     debian/changelog.in \
	     debian/ecore0-bin.install \
	     debian/libecore0.install \
	     debian/control \
	     debian/copyright \
	     debian/rules \
	     debian/libecore0-dev.install

m4datadir = $(datadir)/aclocal
m4data_DATA = ecore.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ecore.pc
