ector_deps = [eina, emile, eet, eo, efl, draw]
ector_pub_deps = [eina, efl]
ector_ext_deps = [triangulator, freetype, draw_without_deps, m]

pub_eo_file_target = []
ector_opt_lib = [ ]


ector_header_src = [
 # nothing for now ector stays only intree
]

ector_src = files([
  'ector_main.c',
  'ector_gl_internal.h',
  'ector_buffer.c',
  'ector_renderer_shape.c',
  'ector_renderer_image.c',
  'ector_renderer.c',
  'ector_renderer_gradient.c',
  'ector_renderer_gradient_radial.c',
  'ector_renderer_gradient_linear.c'
])

pub_eo_files = [
  'ector_surface.eo',
  'ector_buffer.eo',
  'ector_renderer.eo',
  'ector_renderer_shape.eo',
  'ector_renderer_image.eo',
  'ector_renderer_gradient.eo',
  'ector_renderer_gradient_radial.eo',
  'ector_renderer_gradient_linear.eo'
]

ector_pub_eo_files = pub_eo_files

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : false,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-e', 'ECTOR_API',
                           '-gchd', '@INPUT@'])
endforeach

eolian_include_directories += ['-I', meson.current_source_dir()]

subdir('gl')

pub_eo_types_files = [
  'ector_types.eot'
]

#
# Only enable that again when the namespace problems are fixed. ref T8648
#
#if get_option('install-eo-files')
#  install_data(ector_pub_eo_files + pub_eo_types_files,
#    install_dir: join_paths(eolian_include_dir, package_version_name)
#  )
#endif

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : false,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-e', 'ECTOR_API',
                           '-ghd', '@INPUT@'])
endforeach

subdir('software')

ector_opt_lib_dep = declare_dependency(
  link_with: ector_opt_lib
)

ector_ext_deps += ector_opt_lib_dep

ector_lib = library('ector',
    ector_src, pub_eo_file_target,
    c_args : [package_c_args, '-DECTOR_BUILD'],
    dependencies: ector_pub_deps + ector_ext_deps + ector_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version(),
)

ector = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ector_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ector_pub_deps,
)

install_headers(ector_header_src,
  install_dir : dir_package_include,
)

