MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
@Xcursor_cflags@ \
@Xprint_cflags@ \
@Xinerama_cflags@ \
@Xrandr_cflags@ \
@Xss_cflags@ \
@x_cflags@ \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_txt \
-I$(top_srcdir)/src/lib/ecore_job \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_txt \
-I$(top_builddir)/src/lib/ecore_job

libecore_x_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs \
-L$(top_builddir)/src/lib/ecore_txt/.libs \
-L$(top_builddir)/src/lib/ecore_job/.libs

if BUILD_ECORE_X

lib_LTLIBRARIES = libecore_x.la
include_HEADERS = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h

libecore_x_la_SOURCES = \
ecore_x.c \
ecore_x_dnd.c \
ecore_x_sync.c \
ecore_x_randr.c \
ecore_x_error.c \
ecore_x_events.c \
ecore_x_icccm.c \
ecore_x_netwm.c \
ecore_x_mwm.c \
ecore_x_e.c \
ecore_x_selection.c \
ecore_x_window.c \
ecore_x_window_prop.c \
ecore_x_window_shape.c \
ecore_x_pixmap.c \
ecore_x_gc.c \
ecore_x_xinerama.c \
ecore_x_screensaver.c \
ecore_x_private.h

libecore_x_la_LIBADD = \
@Xcursor_libs@ \
@Xprint_libs@ \
@Xinerama_libs@ \
@Xrandr_libs@ \
@Xss_libs@ \
@x_ldflags@ \
@x_libs@ \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la \
$(top_builddir)/src/lib/ecore_job/libecore_job.la

libecore_x_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la \
$(top_builddir)/src/lib/ecore_job/libecore_job.la

endif

EXTRA_DIST = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h \
ecore_x.c \
ecore_x_dnd.c \
ecore_x_sync.c \
ecore_x_randr.c \
ecore_x_error.c \
ecore_x_events.c \
ecore_x_icccm.c \
ecore_x_netwm.c \
ecore_x_mwm.c \
ecore_x_e.c \
ecore_x_selection.c \
ecore_x_window.c \
ecore_x_window_prop.c \
ecore_x_window_shape.c \
ecore_x_pixmap.c \
ecore_x_gc.c \
ecore_x_xinerama.c \
ecore_x_screensaver.c \
ecore_x_private.h
