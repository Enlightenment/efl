engine_src = files([
  'evas_engine.c',
  'evas_engine.h',
  'evas_x_egl.c',
  'evas_x_egl.h',
  'Evas_Engine_Software_X11.h',
  'evas_xlib_swapbuf.c',
  'evas_xlib_outbuf.c',
  'evas_xlib_buffer.c',
  'evas_xlib_color.c',
  'evas_xlib_main.c',
  'evas_xlib_swapper.c',
  'evas_xlib_image.c',
  'evas_xlib_dri_image.c',
  'evas_xlib_outbuf.h',
  'evas_xlib_swapbuf.h',
  'evas_xlib_buffer.h',
  'evas_xlib_color.h',
  'evas_xlib_swapper.h',
  'evas_xlib_image.h',
  'evas_xlib_dri_image.h'
])

#xcb is DEAD ... we only use xlib
config_h.set('BUILD_ENGINE_SOFTWARE_XLIB', '1')

#ecore_x_deps carries all the extensions etc. so we dont have to search them twice
engine_deps = [x11, ecore_x, ecore_x_deps]

if get_option('evas-modules') == 'shared' and not evas_force_static.contains(engine)
  shared_module(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas_pre] + engine_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
endif
