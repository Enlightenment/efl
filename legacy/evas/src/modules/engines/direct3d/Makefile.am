
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_direct3d_cflags@

if BUILD_ENGINE_DIRECT3D

pkgdir = $(libdir)/evas/modules/engines/direct3d/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_direct3d_buffer.c \
evas_direct3d_main.cpp

module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ @evas_engine_direct3d_libs@
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Direct3D.h

endif

EXTRA_DIST = evas_engine.h
