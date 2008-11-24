
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_software_xcb_cflags@

if BUILD_ENGINE_SOFTWARE_XCB

pkgdir = $(libdir)/evas/modules/engines/software_xcb/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_xcb_buffer.c \
evas_xcb_color.c \
evas_xcb_main.c

module_la_LIBADD = @EINA_LIBS@ @evas_engine_software_xcb_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Software_Xcb.h

endif

EXTRA_DIST = evas_engine.h
