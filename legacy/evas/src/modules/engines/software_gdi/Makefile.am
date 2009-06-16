
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_software_gdi_cflags@

if BUILD_ENGINE_SOFTWARE_GDI

SOFTWARE_GDI_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_gdi_buffer.c \
evas_gdi_main.c

SOFTWARE_GDI_LIBADD = @evas_engine_software_gdi_libs@

pkgdir = $(libdir)/evas/modules/engines/software_gdi/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Software_Gdi.h

if !EVAS_STATIC_BUILD_SOFTWARE_GDI

pkg_LTLIBRARIES = module.la
module_la_SOURCES = $(SOFTWARE_GDI_SOURCES)
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ $(SOFTWARE_GDI_LIBADD)
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_gdi.la

libevas_engine_software_gdi_la_SOURCES = $(SOFTWARE_GDI_SOURCES)
libevas_engine_software_gdi_la_LIBADD = $(SOFTWARE_GDI_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
