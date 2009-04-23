
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

pkgdir = $(libdir)/evas/modules/engines/software_gdi/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_gdi_buffer.c \
evas_gdi_main.c

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ @evas_engine_software_gdi_libs@
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Software_Gdi.h

endif

EXTRA_DIST = evas_engine.h
