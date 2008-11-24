
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

pkgdir = $(libdir)/evas/modules/engines/software_sdl/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = evas_engine.c

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ @evas_engine_software_sdl_libs@
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_SDL.h

endif

EXTRA_DIST = evas_engine.h
