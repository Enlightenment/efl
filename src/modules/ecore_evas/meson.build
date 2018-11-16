engines = [
['cocoa', ['cocoa']],
['drm', ['drm']],
['extn', []],
['fb', ['fb']],
['sdl', ['sdl']],
['wayland', ['wl']],
#['win32', ['ecore_win32']],
['x', ['x11']],
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
    mod_install_dir = join_paths(dir_lib, package_name, 'engines', engine, version_name)

    subdir(join_paths('engines', engine))

    module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
  endif
endforeach


if get_option('vnc-server')
  mod_install_dir = join_paths(dir_lib, package_name, engine, version_name)

  subdir(join_paths('vnc_server'))
endif
