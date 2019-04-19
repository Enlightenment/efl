engine_src = files([
  'Evas_Engine_GL_Cocoa.h',
  'evas_engine.c',
  'evas_engine.h',
  'evas_outbuf.m',
])

engine_deps = [ecore_cocoa, gl_deps]

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
