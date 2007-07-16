## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@

noinst_LTLIBRARIES      = libevas_cache.la
libevas_cache_la_SOURCES  = \
evas_cache_image.c \
evas_cache_engine_image.c

libevas_cache_la_DEPENDENCIES = $(top_builddir)/config.h
