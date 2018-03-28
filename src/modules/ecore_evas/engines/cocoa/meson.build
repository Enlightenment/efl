if get_option('opengl') != 'none'
  config_h.set('BUILD_ECORE_EVAS_OPENGL_COCOA', '1')
endif

engine_src = files(['ecore_evas_cocoa.c'])
engine_deps = [ecore_cocoa, ecore_input, ecore_input_evas, ecore]

engine_include_dir += include_directories(join_paths('..', '..', '..', 'evas', 'engines', 'gl_cocoa'))
