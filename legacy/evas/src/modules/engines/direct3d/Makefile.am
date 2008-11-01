
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@

if BUILD_ENGINE_DIRECT3D

pkgdir = $(libdir)/evas/modules/engines/direct3d/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_direct3d_buffer.c \
evas_direct3d_main.cpp

module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = @EINA_LIBS@ $(top_builddir)/src/lib/libevas.la @direct3d_libs@
module_la_LDFLAGS = @lt_no_undefined@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Direct3D.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_direct3d_buffer.c \
evas_direct3d_main.cpp \
Evas_Engine_Direct3D.h
