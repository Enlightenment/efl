## Process this file with automake to produce Makefile.in

SUBDIRS = data src debian examples

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in

dist-hook:
	./gendoc; \
	tar cvf ecore_docs.tar doc/html doc/latex doc/man; \
	rm -f ecore_docs.tar.gz; \
	gzip -9 ecore_docs.tar;

bin_SCRIPTS = ecore-config

EXTRA_DIST = README AUTHORS COPYING ecore.m4 ecore.spec ecore.c.in gendoc \
Doxyfile ecore_docs.tar.gz make_cross_compile_arm.sh

m4datadir = $(datadir)/aclocal
m4data_DATA = ecore.m4
