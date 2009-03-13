
MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_X_XLIB

AM_CPPFLAGS = \
@Xcursor_cflags@ \
@XKB_CFLAGS@ \
@XDAMAGE_CFLAGS@ \
@XCOMPOSITE_CFLAGS@ \
@XDPMS_CFLAGS@ \
@XFIXES_CFLAGS@ \
@XINERAMA_CFLAGS@ \
@XPRINT_CFLAGS@ \
@XRANDR_CFLAGS@ \
@XRENDER_CFLAGS@ \
@XSS_CFLAGS@ \
@XTEST_CFLAGS@ \
@x_cflags@ \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_txt \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_txt \
-I$(top_builddir)/src/lib/ecore_x \
@EINA_CFLAGS@

noinst_LTLIBRARIES = libecore_x_xlib.la

libecore_x_xlib_la_SOURCES = \
ecore_x.c \
ecore_x_dnd.c \
ecore_x_sync.c \
ecore_x_randr.c \
ecore_x_fixes.c \
ecore_x_damage.c \
ecore_x_composite.c \
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
ecore_x_dpms.c \
ecore_x_drawable.c \
ecore_x_cursor.c \
ecore_x_test.c \
ecore_x_atoms.c

libecore_x_xlib_la_LIBADD = \
@Xcursor_libs@ \
@XKB_LIBS@ \
@XDAMAGE_LIBS@ \
@XCOMPOSITE_LIBS@ \
@XDPMS_LIBS@ \
@XFIXES_LIBS@ \
@XINERAMA_LIBS@ \
@XPRINT_LIBS@ \
@XRANDR_LIBS@ \
@XRENDER_LIBS@ \
@XSS_LIBS@ \
@XTEST_LIBS@ \
@x_libs@ \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la \
@EINA_LIBS@

libecore_x_xlib_la_LDFLAGS = -version-info @version_info@

endif

EXTRA_DIST = ecore_x_private.h
