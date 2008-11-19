
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
-I$(top_srcdir)/src/modules/engines/software_16 \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@

.rc.lo:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@

if BUILD_ENGINE_SOFTWARE_16_WINCE

pkgdir = $(libdir)/evas/modules/engines/software_16_wince/$(MODULE_ARCH)

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = \
evas_engine.h \
evas_engine.c \
evas_wince_ddraw_buffer.cpp \
evas_wince_fb_buffer.c \
evas_wince_gapi_buffer.c \
evas_wince_gdi_buffer.c \
evas_wince_gapi.rc

module_la_CFLAGS = @WIN32_CFLAGS@
module_la_CXXFLAGS = -fno-rtti -fno-exceptions
module_la_LIBADD = @EINA_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Software_16_WinCE.h

endif
