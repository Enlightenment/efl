## Process this file with automake to produce Makefile.in

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in eet_docs.tar eet_docs.tar.gz eet.c

bin_SCRIPTS = eet-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN eet.spec eet.c.in \
			gendoc Doxyfile eet_docs.tar.gz eet.pc.in \
			make_cross_compile_arm.sh

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eet.pc

