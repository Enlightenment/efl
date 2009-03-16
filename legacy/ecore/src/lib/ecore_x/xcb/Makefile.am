
MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_X_XCB

AM_CPPFLAGS = \
@XCB_DAMAGE_CFLAGS@ \
@XCB_COMPOSITE_CFLAGS@ \
@XCB_DPMS_CFLAGS@ \
@XCB_RANDR_CFLAGS@ \
@XCB_RENDER_CFLAGS@ \
@XCB_SCREENSAVER_CFLAGS@ \
@XCB_SHAPE_CFLAGS@ \
@XCB_SYNC_CFLAGS@ \
@XCB_XFIXES_CFLAGS@ \
@XCB_XINERAMA_CFLAGS@ \
@XCB_XPRINT_CFLAGS@ \
@XCB_XTEST_CFLAGS@ \
@XCB_CFLAGS@ \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_txt \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_txt \
-I$(top_builddir)/src/lib/ecore_x \
-I$(top_builddir)/src/lib/ecore_input \
@EINA_CFLAGS@

noinst_LTLIBRARIES = libecore_x_xcb.la

libecore_x_xcb_la_SOURCES = \
ecore_xcb_atom.c \
ecore_xcb_cursor.c \
ecore_xcb_damage.c \
ecore_xcb_composite.c \
ecore_xcb_dnd.c \
ecore_xcb_dpms.c \
ecore_xcb_drawable.c \
ecore_xcb_e.c \
ecore_xcb_events.c \
ecore_xcb_fixes.c \
ecore_xcb_gc.c \
ecore_xcb_icccm.c \
ecore_xcb_mwm.c \
ecore_xcb_netwm.c \
ecore_xcb_pixmap.c \
ecore_xcb_randr.c \
ecore_xcb_reply.c \
ecore_xcb_screensaver.c \
ecore_xcb_selection.c \
ecore_xcb_shape.c \
ecore_xcb_sync.c \
ecore_xcb_window.c \
ecore_xcb_window_prop.c \
ecore_xcb_window_shadow.c \
ecore_xcb_xinerama.c \
ecore_xcb.c

libecore_x_xcb_la_LIBADD = \
@XCB_DAMAGE_LIBS@ \
@XCB_COMPOSITE_LIBS@ \
@XCB_DPMS_LIBS@ \
@XCB_RANDR_LIBS@ \
@XCB_RENDER_LIBS@ \
@XCB_SCREENSAVER_LIBS@ \
@XCB_SHAPE_LIBS@ \
@XCB_SYNC_LIBS@ \
@XCB_XFIXES_LIBS@ \
@XCB_XINERAMA_LIBS@ \
@XCB_XPRINT_LIBS@ \
@XCB_XTEST_LIBS@ \
@XCB_LIBS@ \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@EINA_LIBS@

libecore_x_xcb_la_LDFLAGS = -version-info @version_info@

endif

EXTRA_DIST = ecore_xcb_private.h
