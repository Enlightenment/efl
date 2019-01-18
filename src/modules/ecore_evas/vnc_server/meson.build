config_h.set('BUILD_ECORE_EVAS_VNC_SERVER', '1')

engine_src = files([
  'ecore_evas_vnc_server.c',
  'ecore_evas_vnc_server_fb_keymap.c',
  'ecore_evas_vnc_server_fb_keymap.h'
])

engine_deps = [ecore, ecore_input, ecore_evas, evas]
engine_deps += dependency('libvncserver')
engine_available = false

engine_include_dir += include_directories(join_paths('..', '..', '..', 'lib', 'ecore_fb'))

shared_module('vnc_server', engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
