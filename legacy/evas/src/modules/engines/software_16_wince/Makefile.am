
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
-I$(top_srcdir)/src/modules/engines/software_16 \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_software_16_wince_cflags@

if BUILD_ENGINE_SOFTWARE_16_WINCE

SOFTWARE_16_WINCE_SOURCE = \
evas_engine.c \
evas_wince_ddraw_buffer.cpp \
evas_wince_fb_buffer.c \
evas_wince_gapi_buffer.c \
evas_wince_gdi_buffer.c

SOFTWARE_16_WINCE_LIBADD = @evas_engine_software_16_wince_libs@

pkgdir = $(libdir)/evas/modules/engines/software_16_wince/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Software_16_WinCE.h

if !EVAS_STATIC_BUILD_SOFTWARE_16_WINCE

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = $(SOFTWARE_16_WINCE_SOURCE)
module_la_CFLAGS = @WIN32_CFLAGS@
module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = @EINA_LIBS@ $(top_builddir)/src/lib/libevas.la $(SOFTWARE_16_WINCE_LIBADD)
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_16_wince.la

libevas_engine_software_16_wince_la_SOURCES = $(SOFTWARE_16_WINCE_SOURCE)
libevas_engine_software_16_wince_la_LIBADD = $(SOFTWARE_16_WINCE_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
