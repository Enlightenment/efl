ector_deps = [eina, emile, eet, eo, efl]
pub_eo_file_target = []

ector_header_src = [
 # nothing for now ector stays only intree
]

ector_src = [
  'ector_main.c',
  'ector_gl_internal.h',
  'ector_buffer.c',
  'ector_renderer_shape.c',
  'ector_renderer.c',
  'ector_renderer_gradient.c',
  'ector_renderer_gradient_radial.c',
  'ector_renderer_gradient_linear.c'
]

pub_eo_files = [
  'ector_surface.eo',
  'ector_buffer.eo',
  'ector_renderer.eo',
  'ector_renderer_shape.eo',
  'ector_renderer_gradient.eo',
  'ector_renderer_gradient_radial.eo',
  'ector_renderer_gradient_linear.eo'
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-gch', '@INPUT@'])
endforeach

eolian_include_directories += ['-I', meson.current_source_dir()]

subdir('cairo')
subdir('gl')
subdir('software')

pub_eo_types_files = [
  'ector_types.eot'
]

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-gh', '@INPUT@'])
endforeach

ector_lib = library('ector',
    ector_src, pub_eo_file_target,
    dependencies: [triangulator, freetype, draw, m, ector_deps],
    include_directories : config_dir,
    install: true,
)

ector = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ector_lib,
  sources : pub_eo_file_target,
  dependencies: [eina, efl],
)

install_headers(ector_header_src,
  install_dir : dir_package_include,
)