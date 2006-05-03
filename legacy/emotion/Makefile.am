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
	debian/copyright debian/rules debian/libemotion0.install \
	debian/libemotion0-dev.install debian/emotion0-bin.install \
	debian/libemotion0-gstreamer.install debian/libemotion0-xine.install \
	debian/docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emotion.pc
