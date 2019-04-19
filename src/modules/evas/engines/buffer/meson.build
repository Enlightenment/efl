engine_src = files([
  'Evas_Engine_Buffer.h',
  'evas_engine.c',
  'evas_engine.h',
  'evas_outbuf.c'
])

engine_deps = []

evas_include_directories += include_directories('.')

install_headers('Evas_Engine_Buffer.h',
  install_dir : dir_package_include,
)

if get_option('evas-modules') == 'shared'
  shared_module(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas_pre] + engine_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
endif
