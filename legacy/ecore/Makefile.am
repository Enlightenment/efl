## Process this file with automake to produce Makefile.in

SUBDIRS = src debian

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in

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
	tar cvf ecore_docs.tar doc/html doc/latex doc/man; \
	rm -f ecore_docs.tar.gz; \
	gzip -9 ecore_docs.tar; \
	if test -d data; then \
	  mkdir $(distdir)/data; \
	  for file in data/*; do \
	    cp -pR $$file $(distdir)/data; \
	  done \
	fi

bin_SCRIPTS = ecore-config

EXTRA_DIST = README AUTHORS COPYING ecore.m4 ecore.spec ecore.c.in gendoc \
Doxyfile ecore_docs.tar.gz make_cross_compile_arm.sh

m4datadir = $(datadir)/aclocal
m4data_DATA = ecore.m4
