AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include @FREETYPE_CFLAGS@ @SDL_CFLAGS@ @EINA_CFLAGS@

if BUILD_ENGINE_SDL

pkgdir = $(libdir)/evas/modules/engines/software_16_sdl/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = \
evas_engine.c \
evas_engine.h

module_la_LIBADD = @EINA_LIBS@ @SDL_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @lt_no_undefined@ -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs

include_HEADERS = Evas_Engine_SDL_16.h

endif

EXTRA_DIST = \
evas_engine.c \
evas_engine.h \
Evas_Engine_SDL_16.h
