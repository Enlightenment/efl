
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@gl_cflags@

if BUILD_ENGINE_GL_COMMON

noinst_LTLIBRARIES = libevas_engine_gl_common.la
libevas_engine_gl_common_la_SOURCES  = \
evas_gl_private.h \
evas_gl_common.h \
evas_gl_context.c \
evas_gl_font.c \
evas_gl_gradient.c \
evas_gl_image.c \
evas_gl_line.c \
evas_gl_misc.c \
evas_gl_polygon.c \
evas_gl_rectangle.c \
evas_gl_texture.c

libevas_engine_gl_common_la_LIBADD = @gl_libs@ $(top_builddir)/src/lib/libevas.la
libevas_engine_gl_common_la_DEPENDENCIES = $(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_gl_private.h \
evas_gl_context.c \
evas_gl_font.c \
evas_gl_gradient.c \
evas_gl_image.c \
evas_gl_line.c \
evas_gl_misc.c \
evas_gl_polygon.c \
evas_gl_rectangle.c \
evas_gl_texture.c
