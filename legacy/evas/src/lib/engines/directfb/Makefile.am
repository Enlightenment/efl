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
		       @DIRECTFB_CFLAGS@\
		       @freetype_cflags@

if BUILD_ENGINE_DIRECTFB

noinst_LTLIBRARIES      = libevas_engine_directfb.la
libevas_engine_directfb_la_SOURCES  = \
evas_engine_dfb.c \
evas_engine_dfb_image_objects.c \
evas_engine_dfb_image_objects.h \
evas_engine_dfb.h

libevas_engine_directfb_la_LIBADD       = $(LDFLAGS) @DIRECTFB_LIBS@

libevas_engine_directfb_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine_dfb.c \
evas_engine_dfb_image_objects.c \
evas_engine_dfb_image_objects.h \
evas_engine_dfb.h
