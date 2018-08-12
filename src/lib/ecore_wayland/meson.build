ecore_wayland_deps = [ecore_input, ecore, eo, efl, eina]
ecore_wayland_pub_deps = [dependency('wayland-client', version: '>= 1.8.0'), dependency('wayland-cursor', version: '>= 1.8.0 '),  dependency('xkbcommon', version: '>= 0.5.0'), dependency('uuid')]

ecore_wayland_src = [
  'ecore_wl.c',
  'ecore_wl_dnd.c',
  'ecore_wl_input.c',
  'ecore_wl_output.c',
  'ecore_wl_window.c',
  'ecore_wl_subsurf.c',
  'ecore_wl_private.h',
  'subsurface-protocol.c',
  'subsurface-client-protocol.h',
  'ivi-application-protocol.c',
  'ivi-application-client-protocol.h',
  'xdg-shell-client-protocol.h',
  'xdg-shell-protocol.c',
  'session-recovery-client-protocol.h',
  'session-recovery-protocol.c'
]
ecore_wayland_header_src = [
  'Ecore_Wayland.h'
]


ecore_wayland_lib = library('ecore_wayland',
    ecore_wayland_src,
    dependencies: [m] + ecore_wayland_deps + ecore_wayland_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
)

ecore_wayland = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_wayland_lib,
  dependencies: ecore_wayland_pub_deps,
)

install_headers(ecore_wayland_header_src,
  install_dir : dir_package_include,
)
