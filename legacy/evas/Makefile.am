## Process this file with automake to produce Makefile.in

SUBDIRS = data src proj

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in evas_docs.tar evas_docs.tar.gz evas.c

dist-hook:
	./gendoc; \
	tar cvf evas_docs.tar doc/html doc/latex doc/man; \
	rm -f evas_docs.tar.gz; \
	gzip -9 evas_docs.tar;

bin_SCRIPTS = evas-config

EXTRA_DIST = TODO README AUTHORS COPYING COPYING-PLAIN evas.spec evas.c.in gendoc Doxyfile evas_docs.tar.gz evas.pc.in \
make_cross_compile_arm.sh

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evas.pc
