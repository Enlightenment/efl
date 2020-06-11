engines = [
 ['buffer', []],
 ['fb', ['fb']],
 ['drm', ['drm']],
 ['software_x11', ['x11']],
 ['wayland_shm', ['wl']],
]

if sys_windows == true
 engines += [
   ['software_ddraw', []],
   ['software_gdi', []],
 ]
endif

have_gl_engine = false

if get_option('opengl') != 'none'
  engines += [['gl_x11', ['x11']]]
  have_gl_engine = true
endif

if get_option('opengl') == 'full'
  engines += [['gl_cocoa', ['cocoa']]]
  have_gl_engine = true
endif

if get_option('opengl') == 'es-egl'
  engines += [
   ['wayland_egl', ['wl']],
   ['gl_drm', ['drm']]
 ]
  have_gl_engine = true
endif

if have_gl_engine
  engines += [['gl_generic', []]]
endif

foreach engine_conf : engines
  engine = engine_conf[0]
  build = true
  if engine_conf[1].length() > 0
    build = get_option(engine_conf[1][0])
  endif
  if build
    engine_include_dir = []
    engine_src = []
    engine_deps = [eina, evas, evas_ext_none_static_deps, eet]
    engine_dep = declare_dependency(
      include_directories: include_directories(engine),
    )

    var_name = 'engine_'+engine
    set_variable(var_name, engine_dep)

    mod_full_name = engine
    # root meson.build declares the root evas engines project as `evas/engines`,
    # but modules must be installed in evas/modules
    evas_package_modules = join_paths(dir_lib, 'evas', 'modules')
    mod_install_dir = join_paths(evas_package_modules, 'engines', engine, version_name)

    subdir(engine)

    config_h.set('BUILD_ENGINE_'+engine.to_upper(), '1')
  endif
endforeach

config_h.set('EVAS_STATIC_BUILD_SOFTWARE_GENERIC', '1')
config_h.set('BUILD_ENGINE_SOFTWARE_GENERIC', '1')
