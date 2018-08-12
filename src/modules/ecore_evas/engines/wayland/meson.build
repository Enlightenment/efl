if get_option('opengl') == 'es-egl'
  config_h.set('BUILD_ECORE_EVAS_WAYLAND_EGL', '1')
endif

config_h.set('BUILD_ECORE_EVAS_WAYLAND_SHM', '1')

engine_src = files([
  'ecore_evas_wayland_egl.c',
  'ecore_evas_wayland_common.c',
  'ecore_evas_wayland_shm.c',
  'ecore_evas_wayland_private.h'
  ])

engine_deps = [ecore_wl2, ecore_input]
engine_include_dir += include_directories(join_paths('..', '..', '..', 'evas', 'engines', 'wayland_common'))


shared_module(mod_full_name, engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)

ecore_evas_wayland_engine_include_dir = include_directories('.')
