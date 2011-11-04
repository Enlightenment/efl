
MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_X_XLIB

AM_CPPFLAGS = \
@Xcursor_cflags@ \
@XKB_CFLAGS@ \
@XDAMAGE_CFLAGS@ \
@XCOMPOSITE_CFLAGS@ \
@XGESTURE_CFLAGS@ \
@XDPMS_CFLAGS@ \
@XFIXES_CFLAGS@ \
@XI2_CFLAGS@ \
@XINERAMA_CFLAGS@ \
@XPRINT_CFLAGS@ \
@XRANDR_CFLAGS@ \
@XRENDER_CFLAGS@ \
@XSS_CFLAGS@ \
@XTEST_CFLAGS@ \
@x_cflags@ \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_x \
-I$(top_builddir)/src/lib/ecore_input \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

noinst_LTLIBRARIES = libecore_x_xlib.la

libecore_x_xlib_la_SOURCES = \
ecore_x.c \
ecore_x_dnd.c \
ecore_x_sync.c \
ecore_x_randr.c \
ecore_x_randr_11.c \
ecore_x_randr_12.c \
ecore_x_randr_12_edid.c \
ecore_x_randr_13.c \
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
ecore_x_atoms.c \
ecore_x_region.c \
ecore_x_image.c \
ecore_x_xi2.c \
ecore_x_vsync.c \
ecore_x_randr.h \
ecore_x_gesture.c

libecore_x_xlib_la_LIBADD = \
@Xcursor_libs@ \
@XKB_LIBS@ \
@XDAMAGE_LIBS@ \
@XCOMPOSITE_LIBS@ \
@XGESTURE_LIBS@ \
@XDPMS_LIBS@ \
@XFIXES_LIBS@ \
@XI2_LIBS@ \
@XINERAMA_LIBS@ \
@XPRINT_LIBS@ \
@XRANDR_LIBS@ \
@XRENDER_LIBS@ \
@XSS_LIBS@ \
@XTEST_LIBS@ \
@x_libs@ \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@EINA_LIBS@ \
@dlopen_libs@

endif

EXTRA_DIST = ecore_x_private.h
