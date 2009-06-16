
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_directfb_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

if BUILD_ENGINE_DIRECTFB

DIRECTFB_SOURCES = evas_engine.c polygon.c
DIRECTFB_LIBADD = @evas_engine_directfb_libs@

pkgdir = $(libdir)/evas/modules/engines/directfb/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_DirectFB.h

if !EVAS_STATIC_BUILD_DIRECTFB

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = $(DIRECTFB_SOURCES)

module_la_LIBADD = @EINA_LIBS@ $(DIRECTFB_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_directfb.la

libevas_engine_directfb_la_SOURCES = $(DIRECTFB_SOURCES)
libevas_engine_directfb_la_LIBADD = $(DIRECTFB_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
