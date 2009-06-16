
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

if BUILD_ENGINE_SOFTWARE_16

SOFTWARE_16_SOURCES = evas_engine.c

pkgdir = $(libdir)/evas/modules/engines/software_16/$(MODULE_ARCH)

if !EVAS_STATIC_BUILD_SOFTWARE_16

pkg_LTLIBRARIES = module.la
module_la_SOURCES = $(SOFTWARE_16_SOURCES)
module_la_LIBADD = @EINA_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_16.la

libevas_engine_software_16_la_SOURCES = $(SOFTWARE_16_SOURCES)

endif
endif

EXTRA_DIST = \
evas_engine.c
