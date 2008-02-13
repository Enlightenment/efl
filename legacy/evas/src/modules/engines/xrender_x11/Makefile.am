
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@x_cflags@

if BUILD_ENGINE_XRENDER_X11

pkgdir = $(libdir)/evas/modules/engines/xrender_x11/$(MODULE_ARCH)

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = \
evas_engine.h \
evas_engine.c \
evas_engine_ximage.c \
evas_engine_xrender.c \
evas_engine_image.c \
evas_engine_font.c \
evas_engine_gradient.c

module_la_LIBADD = @x_libs@ -lXrender $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
module_la_DEPENDENCIES = $(top_builddir)/config.h

include_HEADERS = Evas_Engine_XRender_X11.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_engine_ximage.c \
evas_engine_xrender.c \
evas_engine_image.c \
evas_engine_font.c \
evas_engine_gradient.c \
Evas_Engine_XRender_X11.h
