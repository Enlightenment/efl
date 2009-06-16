
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

BUFFER_SOURCES = \
evas_engine.c \
evas_outbuf.c

pkgdir = $(libdir)/evas/modules/engines/buffer/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Buffer.h

if !EVAS_STATIC_BUILD_BUFFER

pkg_LTLIBRARIES = module.la

module_la_SOURCES = $(BUFFER_SOURCES)
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@
module_la_LDFLAGS =  -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_buffer.la

libevas_engine_buffer_la_SOURCES = $(BUFFER_SOURCES)

endif
endif

EXTRA_DIST = evas_engine.h
