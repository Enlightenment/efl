## Process this file with automake to produce Makefile.in

SUBDIRS = src debian

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in

dist-hook:
	if test -d data; then \
	  mkdir $(distdir)/data; \
	  for file in data/*; do \
	    if test -f $$file; then \
	      cp -p $$file $(distdir)/data; \
	    fi \
	  done \
	fi

bin_SCRIPTS = ecore-config

EXTRA_DIST = README AUTHORS COPYING ecore.m4

m4datadir = $(datadir)/aclocal
m4data_DATA = ecore.m4
