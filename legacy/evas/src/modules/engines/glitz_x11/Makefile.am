
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_glitz_x11_cflags@

if BUILD_ENGINE_GLITZ_X11

pkgdir = $(libdir)/evas/modules/engines/glitz_x11/$(MODULE_ARCH)

pkg_LTLIBRARIES	= module.la
module_la_SOURCES = \
evas_engine.c \
evas_engine_ximage.c \
evas_engine_glitz.c \
evas_engine_image.c \
evas_engine_font.c \
evas_engine_gradient.c

module_la_LIBADD = @EINA_LIBS@ @evas_engine_glitz_x11_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Glitz_X11.h

endif

EXTRA_DIST = evas_engine.h
