ecore_wl2_deps = []

ecore_wl2_header_src = [
  'Ecore_Wl2.h'
]

ecore_wl2_src = [
  'ecore_wl2_subsurf.c',
  'ecore_wl2_dnd.c',
  'ecore_wl2_window.c',
  'ecore_wl2_input.c',
  'ecore_wl2_output.c',
  'ecore_wl2_display.c',
  'ecore_wl2.c',
  'ecore_wl2_private.h',
  'ecore_wl2_buffer.c',
  'ecore_wl2_surface.c'
]

ecore_wl2_deps += dependency('wayland-client')
ecore_wl2_deps += dependency('wayland-server')
ecore_wl2_deps += dependency('xkbcommon')
ecore_wl2_deps += wayland_protocol
ecore_wl2_deps += dependency('wayland-client')

ecore_wl2_lib = library('ecore_wl2',
    ecore_wl2_src, pub_eo_file_target,
    dependencies: [dl, m, eina, ecore, ecore_input, ecore_wl2_deps, libdrm],
    include_directories : config_dir,
    install: true,
)

ecore_wl2 = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_wl2_lib,
  sources : pub_eo_file_target,
  dependencies: [eina, wayland_protocol],
)

install_headers(ecore_wl2_header_src,
  install_dir : dir_package_include,
)