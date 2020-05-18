engine_src = files([
  'evas_engine.c',
  'evas_outbuf.c',
  'evas_ddraw_buffer.c',
  'evas_ddraw_main.cpp',
])

engine_deps = [cc.find_library('ddraw')]

evas_mod_software_ddraw_c_args = [ '-DEFL_BUILD' ]
evas_mod_software_ddraw_cpp_args = ['-fno-rtti', '-fno-exceptions']

shared_module(mod_full_name, engine_src,
  c_args: evas_mod_software_ddraw_c_args,
  cpp_args: evas_mod_software_ddraw_cpp_args,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
