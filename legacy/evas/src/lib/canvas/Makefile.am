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
		       @freetype_cflags@ @x_cflags@ @DIRECTFB_CFLAGS@

noinst_LTLIBRARIES   = libevas_canvas.la
libevas_canvas_la_SOURCES  = \
evas_callbacks.c \
evas_clip.c \
evas_data.c \
evas_events.c \
evas_layer.c \
evas_main.c \
evas_name.c \
evas_object_gradient.c \
evas_object_image.c \
evas_object_main.c \
evas_object_line.c \
evas_object_polygon.c \
evas_object_rectangle.c \
evas_object_smart.c \
evas_object_text.c \
evas_rectangle.c \
evas_render.c \
evas_smart.c \
evas_stack.c

libevas_canvas_la_LIBADD       = $(LDFLAGS)
libevas_canvas_la_DEPENDENCIES = $(top_builddir)/config.h
