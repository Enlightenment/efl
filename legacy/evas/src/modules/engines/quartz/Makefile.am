
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include @quartz_cflags@ @FREETYPE_CFLAGS@ @EINA_CFLAGS@

if BUILD_ENGINE_QUARTZ

pkgdir = $(libdir)/evas/modules/engines/quartz/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = \
evas_engine.h \
evas_engine.c

module_la_LIBADD = @EINA_LIBS@ @quartz_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Quartz.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
Evas_Engine_Quartz.h
