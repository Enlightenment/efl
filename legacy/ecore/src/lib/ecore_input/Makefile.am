MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_X
ECORE_X_INC = -I$(top_srcdir)/src/lib/ecore_x
else
ECORE_X_INC =
endif

if BUILD_ECORE_FB
ECORE_FB_INC = -I$(top_srcdir)/src/lib/ecore_fb
else
ECORE_FB_INC =
endif

if BUILD_ECORE_DIRECTFB
ECORE_DIRECTFB_INC = -I$(top_srcdir)/src/lib/ecore_directfb -I@DIRECTFB_CFLAGS@
else
ECORE_DIRECTFB_INC =
endif

if BUILD_ECORE_WIN32
ECORE_WIN32_INC = -I$(top_srcdir)/src/lib/ecore_win32
else
ECORE_WIN32_INC =
endif

if BUILD_ECORE_SDL
ECORE_SDL_INC = -I$(top_srcdir)/src/lib/ecore_sdl @SDL_CFLAGS@
else
ECORE_SDL_INC =
endif

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_evas \
$(ECORE_X_INC) \
$(ECORE_FB_INC) \
$(ECORE_DIRECTFB_INC) \
$(ECORE_WIN32_INC) \
$(ECORE_SDL_INC) \
@EVAS_CFLAGS@

if BUILD_ECORE_INPUT

lib_LTLIBRARIES = libecore_input.la
include_HEADERS = Ecore_Input.h

libecore_input_la_SOURCES = \
ecore_input.c

libecore_input_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@EVAS_LIBS@ \
@EVIL_LIBS@

libecore_input_la_LDFLAGS = @lt_enable_auto_import@ -version-info @version_info@

libecore_input_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
ecore_input.c \
Ecore_Input.h

