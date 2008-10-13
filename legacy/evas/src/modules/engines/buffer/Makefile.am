
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

if BUILD_ENGINE_BUFFER

pkgdir = $(libdir)/evas/modules/engines/buffer/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES = \
evas_engine.c \
evas_outbuf.c

module_la_LIBADD = @EINA_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS =  @lt_no_undefined@ @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
module_la_DEPENDENCIES = $(top_builddir)/config.h

include_HEADERS = Evas_Engine_Buffer.h

endif

EXTRA_DIST = evas_engine.h
