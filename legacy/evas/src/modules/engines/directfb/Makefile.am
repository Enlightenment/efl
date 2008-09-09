
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@DIRECTFB_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

if BUILD_ENGINE_DIRECTFB

pkgdir = $(libdir)/evas/modules/engines/directfb/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = evas_engine.c polygon.c

module_la_LIBADD = @DIRECTFB_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @lt_no_undefined@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
module_la_DEPENDENCIES = $(top_builddir)/config.h

include_HEADERS = Evas_Engine_DirectFB.h

endif

EXTRA_DIST = evas_engine.h
