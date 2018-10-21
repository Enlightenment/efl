engine_src = files([
  'evas_engine.c',
  'Evas_Engine_GL_Generic.h',
  'Evas_Engine_GL_Shared.h',
  'evas_ector_gl.h',
  'evas_ector_gl_buffer.c',
  'evas_ector_gl_image_buffer.c',
  join_paths('filters','gl_engine_filter.h'),
  join_paths('filters','gl_filter_blend.c'),
  join_paths('filters','gl_filter_blur.c'),
  join_paths('filters','gl_filter_curve.c'),
  join_paths('filters','gl_filter_displace.c'),
  join_paths('filters','gl_filter_fill.c'),
  join_paths('filters','gl_filter_mask.c'),
])


pub_eo_files = [
  'evas_ector_gl_buffer.eo',
  'evas_ector_gl_image_buffer.eo'
]

foreach eo_file : pub_eo_files
  engine_src += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

engine_deps = [gl_common]

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
