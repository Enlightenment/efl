## Process this file with automake to produce Makefile.in

SUBDIRS = src proj

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in evas_docs.tar evas_docs.tar.gz evas.c

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/data; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/data; \
	  for d in $(srcdir)/data/*; do \
	      cp -pR $$d $(DESTDIR)$(pkgdatadir)/data; \
	  done \
	fi

dist-hook:
	./gendoc; \
	tar cvf evas_docs.tar doc/html doc/latex doc/man; \
	rm -f evas_docs.tar.gz; \
	gzip -9 evas_docs.tar; \
	if test -d data; then \
	  mkdir $(distdir)/data; \
	  for d in data/*; do \
	      cp -pR $$d $(distdir)/data; \
	  done \
	fi

bin_SCRIPTS = evas-config

EXTRA_DIST = TODO README AUTHORS COPYING COPYING-PLAIN evas.spec evas.c.in gendoc Doxyfile evas_docs.tar.gz evas.pc.in \
make_cross_compile_arm.sh

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evas.pc
