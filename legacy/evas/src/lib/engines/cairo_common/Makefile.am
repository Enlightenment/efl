## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = @freetype_cflags@ @CAIRO_CFLAGS@ \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include

if BUILD_ENGINE_CAIRO_COMMON

noinst_LTLIBRARIES      = libevas_engine_cairo_common.la
libevas_engine_cairo_common_la_SOURCES  = \
evas_cairo_private.h \
evas_cairo_main.c

libevas_engine_cairo_common_la_LIBADD       = @CAIRO_LIBS@

libevas_engine_cairo_common_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif
