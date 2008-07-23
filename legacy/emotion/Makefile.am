## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess emotion.spec \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in emotion_docs.tar emotion_docs.tar.gz \
		       emotion.c acconfig.h debian/changelog depcomp

bin_SCRIPTS =

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN \
	emotion.spec emotion.spec.in emotion.c.in gendoc Doxyfile \
	emotion.pc.in debian/changelog debian/control \
	debian/copyright debian/rules debian/libemotion0.install \
	debian/libemotion-dev.install debian/libemotion-bin.install \
	debian/libemotion-gstreamer.install debian/libemotion-xine.install

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emotion.pc
