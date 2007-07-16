MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@sdl_cflags@

libecore_sdl_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_SDL

lib_LTLIBRARIES = libecore_sdl.la
include_HEADERS = \
Ecore_Sdl.h

libecore_sdl_la_SOURCES = \
ecore_sdl.c \
Ecore_Sdl_Keys.h

libecore_sdl_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@sdl_libs@

libecore_sdl_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Sdl.h \
Ecore_Sdl_Keys.h \
ecore_sdl.c
