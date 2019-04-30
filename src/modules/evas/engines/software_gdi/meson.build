engine_src = files([
  'evas_engine.c',
  'evas_outbuf.c',
  'evas_gdi_buffer.c',
  'evas_gdi_main.c',
])

engine_deps = [cc.find_library('gdi32')]

if get_option('evas-modules') == 'shared'
  shared_module(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas] + engine_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
endif
