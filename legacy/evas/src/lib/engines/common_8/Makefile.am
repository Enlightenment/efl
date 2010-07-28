
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@VALGRIND_CFLAGS@ \
@EINA_CFLAGS@ \
@EET_CFLAGS@ \
@pthread_cflags@

noinst_LTLIBRARIES = libevas_engine_common_8.la

libevas_engine_common_8_la_SOURCES = \
evas_soft8_dither_mask.c \
evas_soft8_font.c \
evas_soft8_image_unscaled.c \
evas_soft8_main.c \
evas_soft8_rectangle.c \
evas_soft8_line.c \
evas_soft8_polygon.c \
evas_soft8_image_scaled_sampled.c


libevas_engine_common_8_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST = \
evas_soft8_scanline_blend.c \
evas_soft8_scanline_fill.c
