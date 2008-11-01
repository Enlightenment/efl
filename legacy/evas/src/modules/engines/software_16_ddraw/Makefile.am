
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
-I$(top_srcdir)/src/modules/engines/software_16 \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@

if BUILD_ENGINE_SOFTWARE_16_DDRAW

pkgdir = $(libdir)/evas/modules/engines/software_16_ddraw/$(MODULE_ARCH)

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = \
evas_engine.h \
evas_engine.c \
evas_ddraw_buffer.cpp \
evas_ddraw_main.cpp

module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = @EINA_LIBS@ @ddraw_16_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @lt_no_undefined@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Software_16_DDraw.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_ddraw_buffer.cpp \
evas_ddraw_main.cpp \
Evas_Engine_Software_16_DDraw.h
