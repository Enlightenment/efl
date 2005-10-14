## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@ @x_cflags@ @DIRECTFB_CFLAGS@ \
		       @eet_cflags@ @xcb_cflags@

noinst_LTLIBRARIES   = libevas_canvas.la
libevas_canvas_la_SOURCES  = \
evas_callbacks.c \
evas_clip.c \
evas_data.c \
evas_events.c \
evas_focus.c \
evas_key.c \
evas_key_grab.c \
evas_layer.c \
evas_main.c \
evas_name.c \
evas_object_gradient.c \
evas_object_image.c \
evas_object_main.c \
evas_object_inform.c \
evas_object_intercept.c \
evas_object_line.c \
evas_object_polygon.c \
evas_object_rectangle.c \
evas_object_smart.c \
evas_object_text.c \
evas_object_textblock.c \
evas_object_textblock_old.c \
evas_font_dir.c \
evas_rectangle.c \
evas_render.c \
evas_smart.c \
evas_stack.c

libevas_canvas_la_DEPENDENCIES = $(top_builddir)/config.h
