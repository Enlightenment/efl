engine_src = files([
  'Evas_Engine_GL_Win32.h',
  'evas_engine.c',
  'evas_engine.h',
  'evas_win32_main.c',
])

engine_deps += [
  gl_deps
]

shared_module(mod_full_name, engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
