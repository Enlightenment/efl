## Process this file with automake to produce Makefile.in

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in embryo_docs.tar embryo_docs.tar.gz embryo.c

dist-hook:
	./gendoc; \
	tar cvf embryo_docs.tar doc/html doc/latex doc/man; \
	rm -f embryo_docs.tar.gz; \
	gzip -9 embryo_docs.tar;

bin_SCRIPTS = embryo-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN embryo.spec embryo.c.in \
			gendoc Doxyfile embryo_docs.tar.gz embryo.pc.in \
			make_cross_compile_arm.sh

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = embryo.pc

