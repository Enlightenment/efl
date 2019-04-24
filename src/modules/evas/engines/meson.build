engines = [
 ['buffer', []],
 ['fb', ['fb']],
 ['drm', ['drm']],
 ['software_x11', ['x11']],
 ['wayland_shm', ['wl']],
]

if get_option('opengl') != 'none'
  engines += [
   ['gl_generic', []],
   ['gl_x11', ['x11']],
   ['gl_drm', ['drm']],
   ['gl_cocoa', ['cocoa']],
  ]
endif

if get_option('opengl') == 'es-egl'
  engines += [['wayland_egl', ['wl']]]
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
    engine_deps = []
    engine_dep = declare_dependency(
      include_directories: include_directories(engine),
    )

    var_name = 'engine_'+engine
    set_variable(var_name, engine_dep)

    mod_full_name = engine
    mod_install_dir = join_paths(dir_package_modules, 'engines', engine, version_name)

    subdir(engine)

    if get_option('evas-modules') == 'static'
      tmp = static_library(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas_pre] + engine_deps,
      )
      evas_static_list += declare_dependency(
        include_directories: [include_directories('.')] + config_dir + [engine_include_dir],
        link_with: tmp,
        dependencies : [eina, evas_pre] + engine_deps,
        sources : engine_src
      )
      if engine == 'gl_generic'
        #special case, see evas_module.c
        config_h.set('EVAS_STATIC_BUILD_GL_COMMON', '1')
      else
        config_h.set('EVAS_STATIC_BUILD_'+engine.to_upper(), '1')
      endif
    else
      #nothing here shared building is handled directly on the engine configuration side
      #reason for this is that the .so files have to be placed in the correct directory in order
      # to make them discoverable by evas module code
    endif
    config_h.set('BUILD_ENGINE_'+engine.to_upper(), '1')
  endif
endforeach

config_h.set('EVAS_STATIC_BUILD_SOFTWARE_GENERIC', '1')
config_h.set('BUILD_ENGINE_SOFTWARE_GENERIC', '1')
