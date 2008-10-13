
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

noinst_LTLIBRARIES = libevas_engine_common_16.la

libevas_engine_common_16_la_SOURCES = \
evas_soft16_dither_mask.c \
evas_soft16_font.c \
evas_soft16_image_scaled_sampled.c \
evas_soft16_image_unscaled.c \
evas_soft16_main.c \
evas_soft16_rectangle.c \
evas_soft16_line.c \
evas_soft16_polygon.c

libevas_engine_common_16_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST = \
evas_soft16_scanline_blend.c \
evas_soft16_scanline_fill.c
