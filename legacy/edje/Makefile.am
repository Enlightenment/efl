## Process this file with automake to produce Makefile.in

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in edje_docs.tar edje_docs.tar.gz edje.c

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/data; then \
	 $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/data; \
	  for d in $(srcdir)/data/*; do \
	      cp -pR $$d $(DESTDIR)$(pkgdatadir)/data; \
	  done \
	fi
	if test "x${vimdir}" != "x"; then \
	  $(mkinstalldirs) $(DESTDIR)$(vimdir)/syntax; \
	  cp -p $(srcdir)/data/edc.vim $(DESTDIR)$(vimdir)/syntax/; \
	fi

dist-hook:
	./gendoc; \
	tar cvf edje_docs.tar doc/html doc/latex doc/man; \
	rm -f edje_docs.tar.gz; \
	gzip -9 edje_docs.tar; \
	if test -d data; then \
	  mkdir $(distdir)/data; \
	  for file in data/*; do \
	    cp -pR $$file $(distdir)/data; \
	  done \
	fi

bin_SCRIPTS = edje-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN edje.spec edje.c.in gendoc Doxyfile edje_docs.tar.gz \
make_cross_compile_arm.sh

