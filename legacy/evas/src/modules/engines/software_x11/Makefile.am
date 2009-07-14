
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_software_xlib_cflags@ \
@evas_engine_software_xcb_cflags@

if BUILD_ENGINE_SOFTWARE_X11

SOFTWARE_X11_SOURCES = evas_engine.c

if BUILD_ENGINE_SOFTWARE_XLIB

SOFTWARE_X11_SOURCES += \
evas_xlib_outbuf.c \
evas_xlib_buffer.c \
evas_xlib_color.c \
evas_xlib_main.c

SOFTWARE_X11_LIBADD = @evas_engine_software_xlib_libs@ @evas_engine_software_xcb_libs@

endif

if BUILD_ENGINE_SOFTWARE_XCB

SOFTWARE_X11_SOURCES += \
evas_xcb_outbuf.c \
evas_xcb_buffer.c \
evas_xcb_color.c \
evas_xcb_main.c

endif

pkgdir = $(libdir)/evas/modules/engines/software_x11/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Software_X11.h

if !EVAS_STATIC_BUILD_SOFTWARE_X11

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = $(SOFTWARE_X11_SOURCES)
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ $(SOFTWARE_X11_LIBADD)
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_x11.la

libevas_engine_software_x11_la_SOURCES = $(SOFTWARE_X11_SOURCES)
libevas_engine_software_x11_la_LIBADD = $(SOFTWARE_X11_LIBADD)

endif
endif

EXTRA_DIST = \
evas_engine.h \
evas_xlib_outbuf.h \
evas_xlib_buffer.h \
evas_xlib_color.h \
evas_xcb_outbuf.h \
evas_xcb_buffer.h \
evas_xcb_color.h
