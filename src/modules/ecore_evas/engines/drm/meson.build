if get_option('opengl') != 'none'
  config_h.set('BUILD_ECORE_EVAS_GL_DRM', '1')
endif

engine_src = files(['ecore_evas_drm.c'])
engine_deps = [ecore_drm2, ecore_input, eeze, elput, libdrm, engine_drm]

if get_option('opengl') != 'none'
  config_h.set('BUILD_ECORE_EVAS_GL_DRM', '1')
  engine_deps += [engine_gl_drm]
endif

engine_include_dir += include_directories(join_paths('..', '..', '..', 'evas', 'engines', 'drm'))
