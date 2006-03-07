## Process this file with automake to produce Makefile.in

SUBDIRS = src data

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess emotion.spec \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in emotion_docs.tar emotion_docs.tar.gz \
		       emotion.c acconfig.h debian/changelog

bin_SCRIPTS = emotion-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN \
	emotion.spec emotion.spec.in emotion.c.in gendoc Doxyfile \
	emotion.pc.in debian/changelog debian/control \
	debian/copyright debian/rules debian/libemotion0.files \
	debian/libemotion0-dev.files debian/emotion0-bin.files \
	debian/docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emotion.pc
