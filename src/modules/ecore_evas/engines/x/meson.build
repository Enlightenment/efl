config_h.set('BUILD_ECORE_EVAS_X11', '1')
config_h.set('BUILD_ECORE_EVAS_SOFTWARE_X11', '1')

engine_src = files(['ecore_evas_x.c'])
engine_deps += [ecore_x, ecore_input]

engine_deps += [engine_software_x11]

if get_option('opengl') != 'none'
  config_h.set('BUILD_ECORE_EVAS_OPENGL_X11', '1')
  engine_deps += [engine_gl_x11]
endif

shared_module(mod_full_name, engine_src,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
