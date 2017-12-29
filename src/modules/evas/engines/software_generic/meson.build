engine_src = files([
  'evas_engine.c',
  'Evas_Engine_Software_Generic.h',
  'Evas_Engine_Software_Shared.h',
  'evas_native_tbm.c',
  'evas_native_dmabuf.c',
  'evas_ector_software_buffer.c',
  'evas_native_common.h',
  'evas_ector_software.h',
])


pub_eo_files = [
  'evas_ector_software_buffer.eo'
]

subdir('filters')

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

engine_deps = [draw, dl]
