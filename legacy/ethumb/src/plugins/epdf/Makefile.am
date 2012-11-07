MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/plugins/epdf \
@EFL_ETHUMB_BUILD@ \
@EINA_CFLAGS@ \
@EVAS_CFLAGS@ \
@EPDF_CFLAGS@

pluginsdir = $(libdir)/ethumb/plugins
pkgdir = $(pluginsdir)
pkg_LTLIBRARIES = epdf.la
epdf_la_SOURCES = epdf.c
epdf_la_LIBADD = \
$(top_builddir)/src/lib/libethumb.la \
@EPDF_LIBS@ \
@EVAS_LIBS@ \
@EINA_LIBS@
epdf_la_LDFLAGS = -no-undefined -module -avoid-version
epdf_la_LIBTOOLFLAGS = --tag=disable-static
