
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines/cairo_common \
@FREETYPE_CFLAGS@ \
@evas_engine_cairo_x11_cflags@ \
@EINA_CFLAGS@

if BUILD_ENGINE_CAIRO_X11

pkgdir = $(libdir)/evas/modules/engines/cairo_x11/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = \
evas_engine.c \
evas_x_main.c

module_la_LIBADD = @EINA_LIBS@ @evas_engine_cairo_x11_libs@ $(top_builddir)/src/modules/engines/cairo_common/libevas_engine_cairo_common.la $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Cairo_X11.h

endif

EXTRA_DIST = evas_engine.h
