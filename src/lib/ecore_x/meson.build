ecore_x_deps = [ecore, dl, m, ecore_input]
ecore_x_pub_deps = [eina, efl]

ecore_x_header_src = [
  'Ecore_X.h',
  'Ecore_X_Atoms.h',
  'Ecore_X_Cursor.h'
]

ecore_x_src = [
  'ecore_x_atoms_decl.h',
  'ecore_x.c',
  'ecore_x_dnd.c',
  'ecore_x_sync.c',
  'ecore_x_randr.c',
  'ecore_x_fixes.c',
  'ecore_x_damage.c',
  'ecore_x_composite.c',
  'ecore_x_error.c',
  'ecore_x_events.c',
  'ecore_x_icccm.c',
  'ecore_x_keygrab.c',
  'ecore_x_netwm.c',
  'ecore_x_mwm.c',
  'ecore_x_e.c',
  'ecore_x_selection.c',
  'ecore_x_window.c',
  'ecore_x_window_prop.c',
  'ecore_x_window_shape.c',
  'ecore_x_pixmap.c',
  'ecore_x_present.c',
  'ecore_x_gc.c',
  'ecore_x_xinerama.c',
  'ecore_x_screensaver.c',
  'ecore_x_dpms.c',
  'ecore_x_drawable.c',
  'ecore_x_cursor.c',
  'ecore_x_test.c',
  'ecore_x_atoms.c',
  'ecore_x_region.c',
  'ecore_x_image.c',
  'ecore_x_xi2.c',
  'ecore_x_vsync.c',
  'ecore_x_gesture.c',
  'ecore_x_private.h'
]

ecore_x_checks = [
  ['Xcomposite', 'Xcomposite.h', 'Xcomposite', 'XCompositeQueryExtension'],
  ['Xdamage', 'Xdamage.h', 'Xdamage', 'XDamageSubtract'],
  ['Xdpms', 'dpms.h', 'Xext', 'DPMSQueryExtension'],
  ['Xfixes', 'Xfixes.h', 'Xfixes', 'XFixesExpandRegion'],
  ['Xinerama', 'Xinerama.h', 'Xinerama', 'XineramaQueryScreens'],
  ['Xrandr', 'Xrandr.h', 'Xrandr', 'XRRGetScreenResourcesCurrent'],
  ['Xrender', 'Xrender.h', 'Xrender', 'XRenderFindVisualFormat'],
  ['Xtest', 'XTest.h', 'Xtst', 'XTestFakeKeyEvent'],
  ['Xss', 'scrnsaver.h', 'Xss', 'XScreenSaverSelectInput'],
]

if get_option('xpresent')
  ecore_x_checks += [['Xpresent', 'Xpresent.h', 'Xpresent', 'XPresentQueryExtension']]
endif

if get_option('xgesture')
  ecore_x_checks += [['Xgesture', 'gesture.h', 'Xgesture', 'XGestureQueryExtension']]
endif

if get_option('xinput2')
  ecore_x_checks += [['Xi2', 'XInput2.h', 'Xi', 'XIQueryDevice']]
endif

if get_option('xinput22')
  ecore_x_checks += [['Xi2_2', 'XInput2.h', 'Xi','XIGrabTouchBegin']]
endif

x11 = dependency('x11', required: false)
if x11.found() == false
  x11 = cc.find_library('x11', required: true)
endif

xcb = dependency('xcb', required: false)
if xcb.found() == false
  xcb = cc.find_library('xcb', required: true)
endif

x11_xcb = dependency('x11-xcb', required: false)
if x11_xcb.found() == false
  x11_xcb = cc.find_library('x11-xcb', required: true)
endif

foreach ecore_x_check : ecore_x_checks
  x_lib = cc.find_library(ecore_x_check[2],
    required: false
  )

  if cc.compiles('''
#include <X11/Xlib.h>
#include <X11/extensions/'''+ecore_x_check[1]+'''>
int main () {
    #ifndef '''+ecore_x_check[3]+'''
        '''+ecore_x_check[3]+''';
    #endif
}
    ''',
      dependencies : [x_lib, x11]
    ) == true
    config_h.set('ECORE_'+ecore_x_check[0].to_upper(), '1')
  else
    error('extension '+ecore_x_check[0]+' not found for X11')
  endif
  ecore_x_deps += x_lib
endforeach

ecore_x_deps += x11
ecore_x_deps += xcb
ecore_x_deps += x11_xcb


# there are a few special cases
cc.has_header_symbol('X11/XKBlib.h', 'XkbSetDetectableAutoRepeat',
  dependencies : [x11, xcb]
)
config_h.set('ECORE_XKB', '1')

lib = cc.find_library('Xcursor',
  required: true
)
cc.has_header_symbol('X11/Xcursor/Xcursor.h', 'XcursorSupportsARGB',
  dependencies : [x11, xcb]
  )
config_h.set('ECORE_XCURSOR', '1')
ecore_x_deps += lib

ecore_x_lib = library('ecore_x',
    ecore_x_src, pub_eo_file_target,
    dependencies: ecore_x_pub_deps + ecore_x_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

ecore_x = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_x_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_x_pub_deps,
)

install_headers(ecore_x_header_src,
  install_dir : dir_package_include,
)
