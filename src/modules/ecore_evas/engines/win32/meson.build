config_h.set('BUILD_ECORE_EVAS_SOFTWARE_DDRAW', '1')
config_h.set('BUILD_ECORE_EVAS_SOFTWARE_GDI', '1')

engine_src = files(['ecore_evas_win32.c'])

engine_deps += [engine_software_ddraw, engine_software_gdi]

if get_option('opengl') != 'none'
  config_h.set('BUILD_ECORE_EVAS_OPENGL_WIN32', '1')
  engine_deps += [engine_gl_win32]
endif

shared_module(mod_full_name, engine_src,
  c_args : package_c_args,
  include_directories : config_dir + [engine_include_dir],
  dependencies : [eina, ecore_win32, ecore_input, ecore_evas, ecore_input_evas] + engine_deps,
  install : true,
  install_dir : mod_install_dir,
  name_suffix : sys_mod_extension
)
