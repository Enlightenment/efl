raw_evas_src = [
  'evas_engine.c',
  'Evas_Engine_Software_Generic.h',
  'Evas_Engine_Software_Shared.h',
  'evas_native_tbm.c',
  'evas_native_dmabuf.c',
  'evas_ector_software_buffer.c',
  'evas_native_common.h',
  'evas_ector_software.h',
]

gen_src = []

raw_pub_eo_files = [
  'evas_ector_software_buffer.eo'
]

foreach eo_file : raw_pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file.underscorify(),
    input : join_paths(rel_evas_modules, eo_file),
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', join_paths(meson.current_source_dir(), rel_evas_modules), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

foreach file : raw_evas_src
  evas_src += files(join_paths(rel_evas_modules, file))
endforeach

subdir('filters')
