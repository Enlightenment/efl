
MAINTAINERCLEANFILES = Makefile.in ecore_xcb_keysym_table.h

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
  @XCB_XINPUT_CFLAGS@ \
  @XCB_CURSOR_CFLAGS@ \
  @ECORE_XCB_CFLAGS@ \
  @PIXMAN_CFLAGS@ \
  -I$(top_srcdir)/src/lib/ecore \
  -I$(top_srcdir)/src/lib/ecore_x \
  -I$(top_srcdir)/src/lib/ecore_input \
  -I$(top_builddir)/src/lib/ecore \
  -I$(top_builddir)/src/lib/ecore_x \
  -I$(top_builddir)/src/lib/ecore_input \
  @EINA_CFLAGS@

noinst_LTLIBRARIES = libecore_x_xcb.la

libecore_x_xcb_la_SOURCES = \
  ecore_xcb.c \
  ecore_xcb_atoms.c \
  ecore_xcb_extensions.c \
  ecore_xcb_shape.c \
  ecore_xcb_screensaver.c \
  ecore_xcb_sync.c \
  ecore_xcb_render.c \
  ecore_xcb_randr.c \
  ecore_xcb_xfixes.c \
  ecore_xcb_composite.c \
  ecore_xcb_cursor.c \
  ecore_xcb_damage.c \
  ecore_xcb_dnd.c \
  ecore_xcb_dpms.c \
  ecore_xcb_drawable.c \
  ecore_xcb_e.c \
  ecore_xcb_gc.c \
  ecore_xcb_image.c \
  ecore_xcb_input.c \
  ecore_xcb_mwm.c \
  ecore_xcb_pixmap.c \
  ecore_xcb_region.c \
  ecore_xcb_selection.c \
  ecore_xcb_textlist.c \
  ecore_xcb_events.c \
  ecore_xcb_keymap.c \
  ecore_xcb_netwm.c \
  ecore_xcb_icccm.c \
  ecore_xcb_window.c \
  ecore_xcb_window_prop.c \
  ecore_xcb_window_shape.c \
  ecore_xcb_window_shadow.c \
  ecore_xcb_xinerama.c \
  ecore_xcb_error.c \
  ecore_xcb_xtest.c \
  ecore_xcb_vsync.c \
  ecore_xcb_xdefaults.c

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
  @XCB_XINPUT_LIBS@ \
  @XCB_CURSOR_LIBS@ \
  @ECORE_XCB_LIBS@ \
  @PIXMAN_LIBS@ \
  $(top_builddir)/src/lib/ecore/libecore.la \
  $(top_builddir)/src/lib/ecore_input/libecore_input.la \
  @EINA_LIBS@ \
  @dlopen_libs@

endif

EXTRA_DIST = ecore_xcb_private.h
