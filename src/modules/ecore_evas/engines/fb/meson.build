engine_src = files(['ecore_evas_fb.c'])
engine_deps = [ecore_fb, ecore_input]
engine_include_dir += include_directories(join_paths('..', '..', '..', 'evas', 'engines', 'fb'))

shared_module(mod_full_name, engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
