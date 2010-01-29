
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines/gl_common \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_gl_sdl_cflags@

if BUILD_ENGINE_GL_SDL

GL_SDL_SOURCES = \
evas_engine.c

GL_SDL_LIBADD = @evas_engine_gl_sdl_libs@ $(top_builddir)/src/modules/engines/gl_common/libevas_engine_gl_common.la 

pkgdir = $(libdir)/evas/modules/engines/gl_sdl/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_GL_SDL.h

if !EVAS_STATIC_BUILD_GL_SDL

pkg_LTLIBRARIES = module.la

module_la_SOURCES = $(GL_SDL_SOURCES)
module_la_LIBADD = @EINA_LIBS@ $(GL_SDL_LIBADD) $(top_builddir)/src/lib/libevas.la @dlopen_libs@
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_gl_sdl.la

libevas_engine_gl_sdl_la_SOURCES = $(GL_SDL_SOURCES)
libevas_engine_gl_sdl_la_LIBADD = $(GL_SDL_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
