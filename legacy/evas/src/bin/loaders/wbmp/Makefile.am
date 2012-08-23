
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve2 \
-I$(top_srcdir)/src/bin \
@EVAS_GENERAL_CFLAGS@ \
@EVIL_CFLAGS@

if BUILD_LOADER_WBMP
#if !EVAS_STATIC_BUILD_WBMP

pkgdir = $(libdir)/evas/cserve2/loaders/wbmp/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_wbmp.c

module_la_LIBADD = @EVAS_GENERAL_LIBS@ @EVIL_LIBS@
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

#else

#noinst_LTLIBRARIES = libevas_loader_wbmp.la
#libevas_loader_wbmp_la_SOURCES = evas_image_load_wbmp.c
#libevas_loader_wbmp_la_LIBADD =

#endif
endif
