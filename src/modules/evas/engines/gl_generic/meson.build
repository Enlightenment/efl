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
common_engine_src = [
  'evas_gl_private.h',
  'evas_gl_common.h',
  'evas_gl_define.h',
  'evas_gl_context.c',
  'evas_gl_file_cache.c',
  'evas_gl_shader.c',
  'evas_gl_rectangle.c',
  'evas_gl_texture.c',
  'evas_gl_preload.c',
  'evas_gl_image.c',
  'evas_gl_font.c',
  'evas_gl_polygon.c',
  'evas_gl_line.c',
  'evas_gl_core.c',
  'evas_gl_api_ext.h',
  'evas_gl_api_ext_def.h',
  'evas_gl_core.h',
  'evas_gl_core_private.h',
  'evas_gl_api.c',
  'evas_gl_api_def.h',
  'evas_gl_api_gles1.c',
  'evas_gl_api_gles3_def.h',
  'evas_gl_api_ext.c',
  'evas_gl_3d_common.h',
  'evas_gl_3d_private.h',
  'evas_gl_3d.c',
  'evas_gl_3d_renderer.c',
  'evas_gl_3d_shader.c',
]

foreach src : common_engine_src
  engine_src += files(join_paths('..','gl_common', src))
endforeach

pub_eo_files = [
  'evas_ector_gl_buffer.eo',
  'evas_ector_gl_image_buffer.eo'
]

foreach eo_file : pub_eo_files
  engine_src += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

engine_deps = [gl_deps]

engine_include_dir = include_directories(join_paths('..','software_generic'), join_paths('..', 'gl_common'))

if get_option('evas-modules') == 'shared'
  shared_module(mod_full_name, engine_src,
        include_directories : config_dir + [engine_include_dir],
        dependencies : [eina, evas] + engine_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'lib'+mod_full_name + '.' + sys_mod_extension)
endif
