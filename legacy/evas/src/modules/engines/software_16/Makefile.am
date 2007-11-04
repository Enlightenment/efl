
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@

if BUILD_ENGINE_SOFTWARE_16

pkgdir = $(libdir)/evas/modules/engines/software_16/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES  = \
evas_engine.c \
evas_soft16.h \
evas_soft16_dither_mask.c \
evas_soft16_main.c \
evas_soft16_image_cache.c \
evas_soft16_image_unscaled.c \
evas_soft16_image_scaled_sampled.c \
evas_soft16_font.c \
evas_soft16_rectangle.c \
evas_soft16_polygon.c \
evas_soft16_line.c

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @create_shared_lib@ -module -avoid-version
module_la_DEPENDENCIES = $(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.c \
evas_soft16.h \
evas_soft16_dither_mask.c \
evas_soft16_main.c \
evas_soft16_image_cache.c \
evas_soft16_image_unscaled.c \
evas_soft16_image_scaled_sampled.c \
evas_soft16_font.c \
evas_soft16_rectangle.c \
evas_soft16_polygon.c \
evas_soft16_line.c \
evas_soft16_scanline_fill.c \
evas_soft16_scanline_blend.c

