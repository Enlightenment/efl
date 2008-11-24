
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines/gl_common \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_gl_glew_cflags@

if BUILD_ENGINE_GL_GLEW

pkgdir = $(libdir)/evas/modules/engines/gl_glew/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = \
evas_engine.c \
evas_glew_main.c

module_la_LIBADD = $(top_builddir)/src/modules/engines/gl_common/libevas_engine_gl_common.la $(top_builddir)/src/lib/libevas.la @evas_engine_gl_glew_libs@
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_GL_Glew.h

endif

EXTRA_DIST = evas_engine.h
