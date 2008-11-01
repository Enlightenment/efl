
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines/gl_common \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@x_cflags@ \
@gl_cflags@

if BUILD_ENGINE_GL_X11

pkgdir = $(libdir)/evas/modules/engines/gl_x11/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = \
evas_engine.h \
evas_engine.c \
evas_x_main.c

module_la_LIBADD = @EINA_LIBS@ @gl_libs@ @x_libs@ $(top_builddir)/src/modules/engines/gl_common/libevas_engine_gl_common.la $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_GL_X11.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_x_main.c \
Evas_Engine_GL_X11.h
