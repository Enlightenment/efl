engines = [
['cocoa', ['ecore_cocoa']],
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

    mod_full_name = engine
    mod_install_dir = join_paths(dir_lib, package_name, 'engines', engine, module_arch)

    subdir(join_paths('engines', engine))

    module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
  endif
endforeach
