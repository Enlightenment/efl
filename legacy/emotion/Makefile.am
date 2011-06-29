## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data doc

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess emotion.spec \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in emotion_docs.tar \
		       emotion.c acconfig.h depcomp \
			   $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2

bin_SCRIPTS =

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN \
	emotion.spec emotion.spec.in emotion.c.in Doxyfile \
	emotion.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emotion.pc

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc
