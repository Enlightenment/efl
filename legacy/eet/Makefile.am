## Process this file with automake to produce Makefile.in

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in eet_docs.tar eet_docs.tar.gz eet.c

dist-hook:
	./gendoc; \
	tar cvf eet_docs.tar doc; \
	rm -f eet_docs.tar.gz; \
	gzip -9 eet_docs.tar;

bin_SCRIPTS = eet-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN eet.spec eet.c.in gendoc Doxyfile eet_docs.tar.gz
