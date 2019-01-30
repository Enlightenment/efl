engines = [
 ['buffer', []],
 ['software_generic', []],
 ['fb', ['fb']],
 ['drm', ['drm']],
 ['software_x11', ['x11']],
 ['wayland_shm', ['wl']],
]

if get_option('opengl') != 'none'
  engines += [
   ['gl_common', []],
   ['gl_generic', []],
   ['gl_x11', ['x11']],
   ['gl_drm', ['drm']],
   ['gl_cocoa', ['cocoa']],
  ]
endif

if get_option('opengl') == 'es-egl'
  engines += [['wayland_egl', ['wl']]]
endif

#there are a few modules that should NEVER be build as a module but rather be build as static lib and linked in later
evas_force_static = ['software_generic', 'gl_common']
evas_static_list = []

#fixed dependencies by efl
png = dependency('libpng')
tiff = dependency('libtiff-4')
giflib = cc.find_library('gif')

webp = dependency('libwebp', required: get_option('evas-loaders-disabler').contains('webp') == false)

subdir('image_loaders')
subdir('image_savers')
subdir('model_savers')
subdir('model_loaders')
subdir('vg_savers')
subdir('vg_loaders')

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
      include_directories: include_directories(join_paths('engines', engine)),
    )

    var_name = 'engine_'+engine
    set_variable(var_name, engine_dep)

    mod_full_name = engine
    mod_install_dir = join_paths(dir_package_modules, 'engines', engine, version_name)

  	subdir(join_paths('engines', engine))

    if get_option('evas-modules') == 'static' or evas_force_static.contains(engine)
      tmp = static_library(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas_pre] + engine_deps,
      )

      if engine == 'software_generic'
        software_generic = declare_dependency(
          include_directories: engine_include_dir,
          link_with : tmp
        )
      endif
      if engine == 'gl_common'
        #gl_common will only be used by gl_generic, NOT by libevas.so, so we work arround the static list
        gl_common = declare_dependency(
          include_directories : include_directories(join_paths('engines', 'gl_common')),
          link_with: tmp,
        )
      else
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
      endif
    else
      #nothing here shared building is handled directly on the engine configuration side
      #reason for this is that the .so files have to be placed in the correct directory in order
      # to make them discoverable by evas module code
    endif
    config_h.set('BUILD_ENGINE_'+engine.to_upper(), '1')
  endif
endforeach
