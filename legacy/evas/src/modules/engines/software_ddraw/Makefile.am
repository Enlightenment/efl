
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_software_ddraw_cflags@

if BUILD_ENGINE_SOFTWARE_DDRAW

pkgdir = $(libdir)/evas/modules/engines/software_ddraw/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_ddraw_buffer.c \
evas_ddraw_main.cpp

module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = @EINA_LIBS@ @evas_engine_software_ddraw_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Software_DDraw.h

endif

EXTRA_DIST = evas_engine.h
