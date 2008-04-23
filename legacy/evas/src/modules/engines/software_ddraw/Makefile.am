
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@

if BUILD_ENGINE_SOFTWARE_DDRAW

pkgdir = $(libdir)/evas/modules/engines/software_ddraw/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_ddraw_buffer.c \
evas_ddraw_main.cpp

module_la_LIBADD = @ddraw_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @lt_no_undefined@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
module_la_DEPENDENCIES = $(top_builddir)/config.h

include_HEADERS = Evas_Engine_Software_DDraw.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_ddraw_buffer.c \
evas_ddraw_main.cpp \
Evas_Engine_Software_DDraw.h
