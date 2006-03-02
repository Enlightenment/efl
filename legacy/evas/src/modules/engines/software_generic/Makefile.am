AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include -I$(top_srcdir)/src/modules/engines @FREETYPE_CFLAGS@

pkgdir = $(libdir)/evas/modules/engines/software_generic/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES  = \
evas_engine.c

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
module_la_DEPENDENCIES = $(top_builddir)/config.h
