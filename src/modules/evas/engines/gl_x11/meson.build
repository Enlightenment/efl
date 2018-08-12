engine_src = files([
  'Evas_Engine_GL_X11.h',
  'evas_engine.c',
  'evas_engine.h',
  'evas_x_main.c',
])

engine_deps = [ecore_x_deps, gl_deps, ]

if get_option('evas-modules') == 'shared' and not evas_force_static.contains(engine)
  shared_module(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas_pre] + engine_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
endif
