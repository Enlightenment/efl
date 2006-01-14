## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@ @x_cflags@ @CAIRO_CFLAGS@

if BUILD_ENGINE_CAIRO_X11

pkgdir                               = $(libdir)/evas/modules/engines/cairo_x11/$(MODULE_ARCH)

pkg_LTLIBRARIES         = module.la

module_la_SOURCES  = \
evas_engine.h \
evas_engine.c \
evas_x_main.c

module_la_LIBADD       = @CAIRO_LIBS@ @x_libs@ ../cairo_common/libevas_engine_cairo_common.la
module_la_LDFLAGS      = -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs

module_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_x_main.c
