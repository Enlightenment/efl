engines = [
#['cocoa', ]
['drm', ['ecore_drm2']], 
['extn', []],
['fb', ['ecore_fb']],
#['sdl', ['ecore_sdl']],
['wayland', ['ecore_wl2']],
#['win32', ['ecore_win32']],
['x', ['ecore_x']],
]


foreach engine_conf : engines
  engine = engine_conf[0]
  build = true
  engine_deps = []

  if engine_conf[1].length() > 0 
    build = get_option(engine_conf[1])
  endif

  if build
    engine_include_dir = []
    engine_src = []

    config_h.set('BUILD_ECORE_EVAS_'+engine.to_upper(), '1')

    subdir(join_paths('engines', engine))

    mod_full_name = 'ecore_evas_engine_'+engine
    mod_install_dir = join_paths(dir_lib, package_name, 'engines', engine, module_arch)

    shared_module(mod_full_name, engine_src,
      include_directories : config_dir + [engine_include_dir],
      dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
      install : true,
      install_dir : mod_install_dir
    )
    module_files += join_paths(mod_install_dir, 'lib'+mod_full_name+sys_lib_extension)
  endif
endforeach