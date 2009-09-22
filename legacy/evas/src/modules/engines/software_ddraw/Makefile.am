
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

SOFTWARE_DDRAW_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_ddraw_buffer.c \
evas_ddraw_main.cpp

SOFTWARE_DDRAW_LIBADD = @evas_engine_software_ddraw_libs@

pkgdir = $(libdir)/evas/modules/engines/software_ddraw/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Software_DDraw.h

if !EVAS_STATIC_BUILD_SOFTWARE_DDRAW

pkg_LTLIBRARIES = module.la
module_la_SOURCES = $(SOFTWARE_DDRAW_SOURCES)
module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = @EINA_LIBS@ $(SOFTWARE_DDRAW_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @lt_enable_auto_import@ -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_ddraw.la

libevas_engine_software_ddraw_la_SOURCES = $(SOFTWARE_DDRAW_SOURCES)
libevas_engine_software_ddraw_la_LIBADD = $(SOFTWARE_DDRAW_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
