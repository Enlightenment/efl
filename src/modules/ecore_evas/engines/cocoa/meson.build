if get_option('opengl') != 'none'
  config_h.set('BUILD_ECORE_EVAS_OPENGL_COCOA', '1')
endif

engine_src = files(['ecore_evas_cocoa.c'])
engine_deps = [ecore_cocoa, ecore_input, ecore_input_evas, ecore]

engine_include_dir += include_directories(join_paths('..', '..', '..', 'evas', 'engines', 'gl_cocoa'))

shared_module(mod_full_name, engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
