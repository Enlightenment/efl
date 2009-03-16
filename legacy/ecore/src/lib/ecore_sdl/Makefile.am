MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_input \
@EFL_ECORE_SDL_BUILD@ \
@SDL_CFLAGS@ \
@EINA_CFLAGS@

if BUILD_ECORE_SDL

lib_LTLIBRARIES = libecore_sdl.la
include_HEADERS = \
Ecore_Sdl.h

libecore_sdl_la_SOURCES = \
ecore_sdl.c

libecore_sdl_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@SDL_LIBS@ @EVIL_LIBS@ @EINA_LIBS@

libecore_sdl_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

endif

EXTRA_DIST = Ecore_Sdl_Keys.h
