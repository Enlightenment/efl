eo_deps = []
eo_pub_deps = [eina]

pub_eo_files = [
  'efl_object.eo',
  'efl_class.eo',
  'efl_object_override.eo',
  'efl_interface.eo'
]

eo_src = [
  'eo.c',
  'eo_ptr_indirection.c',
  'eo_ptr_indirection.h',
  'eo_base_class.c',
  'eo_class_class.c',
  'eo_add_fallback.c',
  'eo_add_fallback.h',
  'eo_private.h',
  'eo_internal.h'
]

pub_eo_types_files = [
  'eina_types.eot'
]

eo_header = ['Eo.h']

pub_eo_file_target = []
priv_eo_file_target = []
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

eo_lib = library('eo',
    eo_src, pub_eo_file_target,
    dependencies: [eina, valgrind, dl, execinfo],
    install: true,
    version : meson.project_version()
)

library('eo_dbg',
    eo_src, pub_eo_file_target,
    dependencies: [eina, valgrind, dl, execinfo],
    install: true,
    c_args : '-DEO_DEBUG',
    version : meson.project_version()
)

eo = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: eo_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: eo_deps + eo_pub_deps,
)

install_data(pub_eo_files + pub_eo_types_files,
  install_dir: join_paths(eolian_include_dir, package_version_name)
)

install_headers(eo_header,
  install_dir : dir_package_include,
)
