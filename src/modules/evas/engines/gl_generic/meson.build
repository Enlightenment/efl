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
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-gch', '@INPUT@'])
endforeach

engine_deps = [gl_common]
