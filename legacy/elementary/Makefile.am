SUBDIRS = src data

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in acconfig.h depcomp
		       
EXTRA_DIST = README AUTHORS COPYING autogen.sh eet.pc.in 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = elementary.pc

installed_headersdir = $(prefix)/include/elementary
installed_headers_DATA = elementary_config.h
