## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     = -L/usr/local/lib
INCLUDES	     = \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -I$(includedir) \
                       -I/usr/local/include \
		       @freetype_cflags@

noinst_LTLIBRARIES      = libevas_data.la
libevas_data_la_SOURCES  = \
evas_hash.c \
evas_list.c \
evas_object_list.c

libevas_data_la_LIBADD       = $(LDFLAGS)
libevas_data_la_DEPENDENCIES = $(top_builddir)/config.h
