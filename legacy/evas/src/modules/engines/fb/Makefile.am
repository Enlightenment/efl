
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_fb_cflags@

if BUILD_ENGINE_FB

pkgdir = $(libdir)/evas/modules/engines/fb/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.c \
evas_fb_main.c \
evas_outbuf.c

module_la_LIBADD = @EINA_LIBS@ @evas_engine_fb_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_FB.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_fb.h
