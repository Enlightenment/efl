## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	       = -I. \
			 -I$(top_srcdir)/src/lib \
			 -I$(top_srcdir)/src/lib/include \
			 @FREETYPE_CFLAGS@ @tiff_cflags@

pkgdir                 = $(libdir)/evas/modules/loaders/tiff/$(MODULE_ARCH)

pkg_LTLIBRARIES        = module.la

module_la_SOURCES      = evas_image_load_tiff.c

module_la_LIBADD       = @tiff_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS      = -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs

module_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST             = evas_image_load_tiff.c
