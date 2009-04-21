MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/plugins/epdf \
	@EINA_CFLAGS@ @EVAS_CFLAGS@ @ECORE_CFLAGS@ @EPDF_CFLAGS@

pkgdir = $(pluginsdir)
pkg_LTLIBRARIES = epdf.la
epdf_la_SOURCES = epdf.c
epdf_la_DEPENDENCIES = $(top_builddir)/config.h
epdf_la_LIBADD = $(top_builddir)/src/lib/libethumb.la \
	@EINA_LIBS@ @EVAS_LIBS@ @ECORE_LIBS@ @EPDF_LIBS@
epdf_la_LDFLAGS = -module -avoid-version
