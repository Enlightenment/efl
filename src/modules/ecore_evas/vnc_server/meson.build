config_h.set('BUILD_ECORE_EVAS_VNC_SERVER', '1')

engine_src = files([
  'ecore_evas_vnc_server.c',
  'ecore_evas_vnc_server_fb_keymap.c',
  'ecore_evas_vnc_server_fb_keymap.h'
])

engine_deps = [ecore, ecore_input, ecore_evas, evas]
engine_available = false

if get_option('x11')
  engine_deps += [ecore_x]
  engine_available = true
endif

if get_option('fb')
  engine_deps += [ecore_fb]
  engine_available = true
endif

engine_include_dir += include_directories(join_paths('..', '..', '..', 'lib', 'ecore_fb'))

if engine_available == false
  error('at least x11 or fb needs to be build to build vnc')
endif

shared_module(mod_full_name, engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
