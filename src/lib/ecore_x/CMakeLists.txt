set(PUBLIC_LIBRARIES
  eina
  ecore
)

set(LIBRARIES
  ecore
  ecore_input
  efl
  eo
  ${X_EXTENSIONS}
)

message("${LIBRARIES}")

#todo XRANDR_GOOD

set(PUBLIC_HEADERS
  Ecore_X.h
 # ecore_x_version.h
  Ecore_X_Atoms.h
  Ecore_X_Cursor.h
)

set(SOURCES
  ecore_x_atoms_decl.h
  ecore_x.c
  ecore_x_dnd.c
  ecore_x_sync.c
  ecore_x_randr.c
  ecore_x_fixes.c
  ecore_x_damage.c
  ecore_x_composite.c
  ecore_x_error.c
  ecore_x_events.c
  ecore_x_icccm.c
  ecore_x_keygrab.c
  ecore_x_netwm.c
  ecore_x_mwm.c
  ecore_x_e.c
  ecore_x_selection.c
  ecore_x_window.c
  ecore_x_window_prop.c
  ecore_x_window_shape.c
  ecore_x_pixmap.c
  ecore_x_present.c
  ecore_x_gc.c
  ecore_x_xinerama.c
  ecore_x_screensaver.c
  ecore_x_dpms.c
  ecore_x_drawable.c
  ecore_x_cursor.c
  ecore_x_test.c
  ecore_x_atoms.c
  ecore_x_region.c
  ecore_x_image.c
  ecore_x_xi2.c
  ecore_x_vsync.c
  ecore_x_gesture.c
  ecore_x_private.h
)
