
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

GL_GLEW_SOURCES = \
evas_engine.c \
evas_glew_main.c

GL_GLEW_LIBADD = @evas_engine_gl_glew_libs@ $(top_builddir)/src/modules/engines/gl_common/libevas_engine_gl_common.la

pkgdir = $(libdir)/evas/modules/engines/gl_glew/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_GL_Glew.h

if !EVAS_STATIC_BUILD_GL_GLEW

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = $(GL_GLEW_SOURCES)
module_la_LIBADD = $(GL_GLEW_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_gl_glew.la

libevas_engine_gl_glew_la_SOURCES = $(GL_GLEW_SOURCES)
libevas_engine_gl_glew_la_LIBADD = $(GL_GLEW_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
