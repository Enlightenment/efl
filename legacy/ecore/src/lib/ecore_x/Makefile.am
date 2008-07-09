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
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_txt
else
AM_CPPFLAGS = \
@Xcursor_cflags@ \
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
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_txt
endif

if BUILD_ECORE_X_XCB

lib_LTLIBRARIES = libecore_x.la
include_HEADERS = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h

libecore_x_la_SOURCES = \
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
ecore_xcb_xinerama.c \
ecore_xcb.c \
ecore_xcb_private.h \
ecore_x_atoms_decl.h

libecore_x_la_LIBADD = \
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
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la

libecore_x_la_LDFLAGS = -version-info @version_info@

libecore_x_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la

else

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
ecore_x_private.h \
ecore_x_atoms_decl.h

libecore_x_la_LIBADD = \
@Xcursor_libs@ \
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
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la

libecore_x_la_LDFLAGS = -version-info @version_info@

libecore_x_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la

endif

endif

EXTRA_DIST = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h \
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
ecore_x_atoms.c \
ecore_x_private.h \
ecore_x_atoms_decl.h
