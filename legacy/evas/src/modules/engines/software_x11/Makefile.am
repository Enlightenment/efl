
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_software_x11_cflags@ \
@evas_engine_software_xcb_cflags@

if BUILD_ENGINE_SOFTWARE_X11

pkgdir = $(libdir)/evas/modules/engines/software_x11/$(MODULE_ARCH)

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = \
evas_engine.c \
evas_xlib_outbuf.c \
evas_xlib_buffer.c \
evas_xlib_color.c \
evas_xlib_main.c

if BUILD_ENGINE_SOFTWARE_XCB

module_la_SOURCES += \
evas_xcb_outbuf.c \
evas_xcb_buffer.c \
evas_xcb_color.c \
evas_xcb_main.c

endif

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ @evas_engine_software_x11_libs@ @evas_engine_software_xcb_libs@
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Software_X11.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_xlib_outbuf.h \
evas_xlib_buffer.h \
evas_xlib_color.h \
evas_xcb_outbuf.h \
evas_xcb_buffer.h \
evas_xcb_color.h
