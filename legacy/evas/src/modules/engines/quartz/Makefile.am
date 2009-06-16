
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@evas_engine_quartz_cflags@ \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@

if BUILD_ENGINE_QUARTZ

QUARTZ_SOURCES = evas_engine.c
QUARTZ_LIBADD = @evas_engine_quartz_libs@

pkgdir = $(libdir)/evas/modules/engines/quartz/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Quartz.h

if !EVAS_STATIC_BUILD_QUARTZ

pkg_LTLIBRARIES = module.la

module_la_SOURCES = $(QUARTZ_SOURCES)
module_la_LIBADD = @EINA_LIBS@ $(QUARTZ_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_quartz.la

libevas_engine_quartz_la_SOURCES = $(QUARTZ_SOURCES)
libevas_engine_quartz_la_LIBADD = $(QUARTZ_LIBADD)

endif
endif

EXTRA_DIST = \
evas_engine.h \
evas_quartz_private.h
