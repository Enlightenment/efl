MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_X
ECORE_X_INC = -I$(top_srcdir)/src/lib/ecore_x @x_cflags@ @XCB_CFLAGS@
ECORE_X_LIB = $(top_builddir)/src/lib/ecore_x/libecore_x.la @x_libs@ @XCB_LIBS@
else
ECORE_X_INC =
ECORE_X_LIB =
endif

if BUILD_ECORE_FB
ECORE_FB_INC = -I$(top_srcdir)/src/lib/ecore_fb
ECORE_FB_LIB = $(top_builddir)/src/lib/ecore_fb/libecore_fb.la
else
ECORE_FB_INC =
ECORE_FB_LIB =
endif

if BUILD_ECORE_DIRECTFB
ECORE_DIRECTFB_INC = -I$(top_srcdir)/src/lib/ecore_directfb -I@DIRECTFB_CFLAGS@
ECORE_DIRECTFB_LIB = $(top_builddir)/src/lib/ecore_directfb/libecore_directfb.la
else
ECORE_DIRECTFB_INC =
ECORE_DIRECTFB_LIB =
endif

if BUILD_ECORE_WIN32
ECORE_WIN32_INC = -I$(top_srcdir)/src/lib/ecore_win32
ECORE_WIN32_LIB = $(top_builddir)/src/lib/ecore_win32/libecore_win32.la
else
ECORE_WIN32_INC =
ECORE_WIN32_LIB =
endif

if BUILD_ECORE_SDL
ECORE_SDL_INC = -I$(top_srcdir)/src/lib/ecore_sdl @SDL_CFLAGS@
ECORE_SDL_LIB = $(top_builddir)/src/lib/ecore_sdl/libecore_sdl.la
ECORE_SDL_LIBADD = @SDL_LIBS@ $(ECORE_SDL_LIB)
else
ECORE_SDL_INC =
ECORE_SDL_LIB =
ECORE_SDL_LIBADD =
endif

if BUILD_ECORE_WINCE
ECORE_WINCE_INC = -I$(top_srcdir)/src/lib/ecore_wince
ECORE_WINCE_LIB = $(top_builddir)/src/lib/ecore_wince/libecore_wince.la
else
ECORE_WINCE_INC =
ECORE_WINCE_LIB =
endif

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_input_evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_input_evas \
@EFL_ECORE_EVAS_BUILD@ \
$(ECORE_X_INC) \
$(ECORE_FB_INC) \
$(ECORE_DIRECTFB_INC) \
$(ECORE_WIN32_INC) \
$(ECORE_SDL_INC) \
$(ECORE_WINCE_INC) \
@EVAS_CFLAGS@ \
@XCB_CFLAGS@ \
@EINA_CFLAGS@ \
@EVIL_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

lib_LTLIBRARIES = libecore_evas.la
includes_HEADERS = Ecore_Evas.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_evas_la_SOURCES = \
ecore_evas.c \
ecore_evas_util.c \
ecore_evas_x.c \
ecore_evas_fb.c \
ecore_evas_buffer.c \
ecore_evas_directfb.c \
ecore_evas_win32.c \
ecore_evas_sdl.c \
ecore_evas_wince.c

libecore_evas_la_LIBADD = \
$(ECORE_X_LIB) \
$(ECORE_FB_LIB) \
$(ECORE_DIRECTFB_LIB) \
$(ECORE_WIN32_LIB) \
$(ECORE_SDL_LIB) \
$(ECORE_SDL_LIBADD) \
$(ECORE_WINCE_LIB) \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_input_evas/libecore_input_evas.la \
$(top_builddir)/src/lib/ecore/libecore.la \
@EVAS_LIBS@ \
@EINA_LIBS@ \
@EVIL_LIBS@

libecore_evas_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = \
ecore_evas_private.h
