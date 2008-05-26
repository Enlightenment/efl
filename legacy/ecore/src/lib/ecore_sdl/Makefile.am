MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@SDL_CFLAGS@

if BUILD_ECORE_SDL

lib_LTLIBRARIES = libecore_sdl.la
include_HEADERS = \
Ecore_Sdl.h

libecore_sdl_la_SOURCES = \
ecore_sdl.c \
Ecore_Sdl_Keys.h

libecore_sdl_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@SDL_LIBS@

libecore_sdl_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@

libecore_sdl_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Sdl.h \
Ecore_Sdl_Keys.h \
ecore_sdl.c
