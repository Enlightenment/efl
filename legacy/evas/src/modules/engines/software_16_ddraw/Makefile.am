
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
-I$(top_srcdir)/src/modules/engines/software_16 \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_software_16_ddraw_cflags@

if BUILD_ENGINE_SOFTWARE_16_DDRAW

SOFTWARE_16_DDRAW_SOURCES = \
evas_engine.c \
evas_ddraw_buffer.cpp \
evas_ddraw_main.cpp

SOFTWARE_16_DDRAW_LIBADD = @evas_engine_software_16_ddraw_libs@

pkgdir = $(libdir)/evas/modules/engines/software_16_ddraw/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Software_16_DDraw.h

if !EVAS_STATIC_BUILD_SOFTWARE_16_DDRAW

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = $(SOFTWARE_16_DDRAW_SOURCES)
module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ $(SOFTWARE_16_DDRAW_LIBADD)
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_16_ddraw.la

libevas_engine_software_16_ddraw_la_SOURCES = $(SOFTWARE_16_DDRAW_SOURCES)
libevas_engine_software_16_ddraw_la_LIBADD = $(SOFTWARE_16_DDRAW_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
