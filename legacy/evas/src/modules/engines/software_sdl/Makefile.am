
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_software_sdl_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

if BUILD_ENGINE_SOFTWARE_SDL

SOFTWARE_SDL_SOURCES = evas_engine.c
SOFTWARE_SDL_LIBADD = @evas_engine_software_sdl_libs@


includes_HEADERS = Evas_Engine_SDL.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_SOFTWARE_SDL

pkgdir = $(libdir)/evas/modules/engines/software_sdl/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES  = $(SOFTWARE_SDL_SOURCES)
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ $(SOFTWARE_SDL_LIBADD)
module_la_LDFLAGS = @lt_enable_auto_import@ -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_sdl.la

libevas_engine_software_sdl_la_SOURCES = $(SOFTWARE_SDL_SOURCES)
libevas_engine_software_sdl_la_LIBADD = $(SOFTWARE_SDL_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
