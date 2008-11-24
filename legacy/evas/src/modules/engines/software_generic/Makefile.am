
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

pkgdir = $(libdir)/evas/modules/engines/software_generic/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = evas_engine.c

module_la_LIBADD = @EINA_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
