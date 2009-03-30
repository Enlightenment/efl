MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	@EVAS_CFLAGS@ @ECORE_EVAS_CFLAGS@ @ECORE_FILE_CFLAGS@ @EDJE_CFLAGS@

include_HEADERS = Ethumb.h
noinst_HEADERS = md5.h

lib_LTLIBRARIES = libethumb.la


libethumb_la_SOURCES = \
	Ethumb.c \
	md5.c
libethumb_la_DEPENDENCIES = $(top_builddir)/config.h
libethumb_la_LIBADD = \
	@EVAS_LIBS@ @ECORE_EVAS_LIBS@ @ECORE_FILE_LIBS@ @EDJE_LIBS@
libethumb_la_LDFLAGS = -version-info @version_info@
