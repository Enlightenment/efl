
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_xrender_x11_cflags@ \
@evas_engine_xrender_xcb_cflags@

if BUILD_ENGINE_XRENDER_X11

pkgdir = $(libdir)/evas/modules/engines/xrender_x11/$(MODULE_ARCH)

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = \
evas_engine_xlib_font.c \
evas_engine_xlib_gradient.c \
evas_engine_xlib_image.c \
evas_engine_xlib_render.c \
evas_engine_xlib_ximage.c \
evas_engine.c

if BUILD_ENGINE_XRENDER_XCB
module_la_SOURCES += \
evas_engine_xcb_font.c \
evas_engine_xcb_gradient.c \
evas_engine_xcb_image.c \
evas_engine_xcb_render.c \
evas_engine_xcb_ximage.c
endif

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ @evas_engine_xrender_xcb_libs@ @evas_engine_xrender_x11_libs@
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_XRender_X11.h

endif

EXTRA_DIST = evas_engine.h
