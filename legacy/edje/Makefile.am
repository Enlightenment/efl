## Process this file with automake to produce Makefile.in

SUBDIRS = data src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in edje_docs.tar edje_docs.tar.gz edje.c

### this is all fine and great - ut edje should not be going and installing
### data in another software's data dir - it's just wrong. not to mention this
### also breaks make distcheck...
#	if test "x${vimdir}" != "x"; then \
#	  $(mkinstalldirs) $(DESTDIR)$(vimdir)/syntax; \
#	  cp -p $(srcdir)/data/edc.vim $(DESTDIR)$(vimdir)/syntax/; \
#	fi

dist-hook:
	./gendoc; \
	tar cvf edje_docs.tar doc/html doc/latex doc/man; \
	rm -f edje_docs.tar.gz; \
	gzip -9 edje_docs.tar;

bin_SCRIPTS = edje-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN edje.spec edje.pc.in edje.c.in gendoc Doxyfile edje_docs.tar.gz

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = edje.pc
