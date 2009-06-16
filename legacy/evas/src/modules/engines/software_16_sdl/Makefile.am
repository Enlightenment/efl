AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include @FREETYPE_CFLAGS@ @SDL_CFLAGS@ @EINA_CFLAGS@

if BUILD_ENGINE_SOFTWARE_SDL

SOFTWARE_SDL_SOURCES = \
evas_engine.c \
evas_engine.h

pkgdir = $(libdir)/evas/modules/engines/software_16_sdl/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_SDL_16.h

if !EVAS_STATIC_BUILD_SOFTWARE_SDL

pkg_LTLIBRARIES = module.la

module_la_SOURCES = $(SOFTWARE_SDL_SOURCES)

module_la_LIBADD = @EINA_LIBS@ @SDL_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs

else

noinst_LTLIBRARIES = libevas_engine_software_16_sdl.la

libevas_engine_software_16_sdl_la_SOURCES = $(SOFTWARE_SDL_SOURCES)
libevas_engine_software_16_sdl_la_LIBADD = @SDL_LIBS@

endif
endif

EXTRA_DIST = \
evas_engine.c \
evas_engine.h \
Evas_Engine_SDL_16.h
