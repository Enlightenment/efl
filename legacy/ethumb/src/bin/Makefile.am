MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	@EINA_CFLAGS@ @EVAS_CFLAGS@ @ECORE_CFLAGS@ @ECORE_EVAS_CFLAGS@ \
	@EDJE_CFLAGS@ @ECORE_FILE_CFLAGS@

bin_PROGRAMS = ethumb

ethumb_SOURCES = ethumb.c
ethumb_LDADD = \
	@EINA_LIBS@ @EVAS_LIBS@ @ECORE_LIBS@ @ECORE_EVAS_LIBS@ @EDJE_LIBS@ \
	@ECORE_FILE_LIBS@ \
	$(top_builddir)/src/lib/libethumb.la
ethumb_DEPENDENCIES = $(top_builddir)/config.h
