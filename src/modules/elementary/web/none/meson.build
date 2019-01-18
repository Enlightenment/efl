pub_eo_files = [
  'elm_web_none.eo'
]


foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach


src = files([
  'elm_web_none.c'
]) + pub_eo_file_target

shared_module('none',
    src,
    dependencies: [elementary, elementary_deps],
    install: true,
    install_dir : mod_install_dir
)

mod = 'none'
