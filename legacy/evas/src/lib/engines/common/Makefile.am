## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     = -L/usr/local/lib
INCLUDES	     = @freetype_cflags@ \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -I$(includedir) \
                       -I/usr/local/include

noinst_LTLIBRARIES      = libevas_engine_common.la
libevas_engine_common_la_SOURCES  = \
evas_blend_alpha_color_pixel.c \
evas_blend_color_pixel.c \
evas_blend_main.c \
evas_blend_pixel_cmod_pixel.c \
evas_blend_pixel_mul_pixel.c \
evas_blend_pixel_pixel.c \
evas_blit_main.c \
evas_convert_color.c \
evas_convert_gry_1.c \
evas_convert_gry_4.c \
evas_convert_gry_8.c \
evas_convert_main.c \
evas_convert_rgb_16.c \
evas_convert_rgb_24.c \
evas_convert_rgb_32.c \
evas_convert_rgb_8.c \
evas_cpu.c \
evas_draw_main.c \
evas_font_draw.c \
evas_font_load.c \
evas_font_main.c \
evas_font_query.c \
evas_gradient_main.c \
evas_image_load.c \
evas_image_main.c \
evas_line_main.c \
evas_polygon_main.c \
evas_rectangle_main.c \
evas_scale_main.c \
evas_scale_sample.c \
evas_scale_smooth.c \
evas_tiler.c

EXTRA_DIST = evas_scale_smooth_scaler.c

libevas_engine_common_la_LIBADD       = \
$(LDFLAGS)

libevas_engine_common_la_DEPENDENCIES = \
$(top_builddir)/config.h
