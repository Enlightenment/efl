## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = @freetype_cflags@ \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include

noinst_LTLIBRARIES      = libevas_imaging.la
libevas_imaging_la_SOURCES  = \
evas_imaging.c

libevas_imaging_la_LIBADD       = $LDFLAGS
libevas_imaging_la_DEPENDENCIES = $(top_builddir)/config.h
