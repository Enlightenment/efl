engine_src = files([
  'Evas_Engine_GL_Drm.h',
  'evas_engine.c',
  'evas_engine.h',
  'evas_outbuf.c',
])

engine_deps = [ecore_drm2, libdrm, gl_deps, dependency('gbm')]

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
