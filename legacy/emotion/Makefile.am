## Process this file with automake to produce Makefile.in

SUBDIRS = src data

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in emotion_docs.tar emotion_docs.tar.gz \
		       emotion.c acconfig.h

bin_SCRIPTS = emotion-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN emotion.spec emotion.c.in gendoc Doxyfile emotion.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emotion.pc
