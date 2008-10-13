
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@ @CAIRO_CFLAGS@ @EINA_CFLAGS@

if BUILD_ENGINE_CAIRO_COMMON

noinst_LTLIBRARIES      = libevas_engine_cairo_common.la
libevas_engine_cairo_common_la_SOURCES  = \
evas_cairo_private.h \
evas_cairo_common.h \
evas_cairo_main.c

libevas_engine_cairo_common_la_LIBADD       = @EINA_LIBS@ @CAIRO_LIBS@ $(top_builddir)/src/lib/libevas.la

libevas_engine_cairo_common_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif
