
ector_src += files([
  'ector_renderer_software_gradient_linear.c',
  'ector_renderer_software_gradient_radial.c',
  'ector_renderer_software_shape.c',
  'ector_software_gradient.c',
  'ector_software_rasterizer.c',
  'ector_software_surface.c',
  'ector_software_buffer.c',
])

pub_eo_files = [
  'ector_software_surface.eo',
  'ector_software_buffer.eo',
  'ector_software_buffer_base.eo',
  'ector_renderer_software.eo',
  'ector_renderer_software_shape.eo',
  'ector_renderer_software_gradient_radial.eo',
  'ector_renderer_software_gradient_linear.eo',
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : false,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

install_data(pub_eo_files,
  install_dir: join_paths(eolian_include_dir, package_version_name)
)
