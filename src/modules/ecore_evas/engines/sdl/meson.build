engine_src = files(['ecore_evas_sdl.c'])
engine_deps = [ecore_sdl, ecore_input, ecore, dependency('sdl2')]

shared_module(mod_full_name, engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
