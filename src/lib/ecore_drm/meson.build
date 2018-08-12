ecore_drm_deps = [eeze, eldbus, ecore_input, ecore, eo, efl, eina, m]
ecore_drm_pub_deps = [dependency('libinput', version: '>= 1.6.0'), dependency('libdrm', version: '>= 2.4'), dependency('wayland-cursor', version: '>= 1.8.0 '),  dependency('xkbcommon', version: '>= 0.3.0'), dependency('gbm')]

ecore_drm_src = [
  'ecore_drm_sprites.c',
  'ecore_drm_fb.c',
  'ecore_drm_evdev.c',
  'ecore_drm_inputs.c',
  'ecore_drm_output.c',
  'ecore_drm_tty.c',
  'ecore_drm_device.c',
  'ecore_drm_launcher.c',
  'ecore_drm_dbus.c',
  'ecore_drm_logind.c',
  'ecore_drm.c',
  'ecore_drm_private.h'
]
ecore_drm_header_src = [
  'Ecore_Drm.h'
]


ecore_drm_lib = library('ecore_drm',
    ecore_drm_src,
    dependencies: [m] + ecore_drm_deps + ecore_drm_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
)

ecore_drm = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_drm_lib,
  dependencies: ecore_drm_pub_deps,
)

install_headers(ecore_drm_header_src,
  install_dir : dir_package_include,
)
