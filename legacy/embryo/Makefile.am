## Process this file with automake to produce Makefile.in

SUBDIRS = src include examples

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in build-stamp configure-stamp depcomp \
		       embryo_docs.tar.gz embryo.c \
		       README \
		       embryo.pc \
		       embryo.spec \
		       embryo.oe embryo-native.oe \
		       embryo.bb embryoXnative.bb \
		       debian/changelog

bin_SCRIPTS = embryo-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN embryo.c.in gendoc \
             Doxyfile \
	     README.in README \
	     embryo.pc.in \
	     embryo.spec.in embryo.spec \
	     embryo.oe.in embryo.oe embryo-native.oe.in embryo-native.oe \
	     embryo.bb.in embryo.bb embryoXnative.bb.in embryoXnative.bb \
	     debian/changelog \
	     debian/compat \
	     debian/control \
	     debian/copyright \
	     debian/docs \
	     debian/embryo0-bin.files \
	     debian/libembryo0-dev.files \
	     debian/libembryo0.files \
	     debian/rules

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = embryo.pc

