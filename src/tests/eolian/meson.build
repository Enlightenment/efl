priv_eo_files = [
   'generated_future.eo'
]

eolian_test_src = [
'eolian_parsing.c',
'eolian_generation.c',
'eolian_generated_future.c',
'eolian_suite.c',
'eolian_suite.h',
'eolian_static.c'
]

priv_eo_file_target = []
foreach eo_file : priv_eo_files
  priv_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-gch', '@INPUT@'])
endforeach

eolian_suite = executable('eolian_suite',
  eolian_test_src, priv_eo_file_target,
  #FIXME eolian
  dependencies: [eolian, check, eo],
  c_args : [
  '-DEOLIAN_GEN="'+eolian_gen.full_path()+'"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DEO_SRC_DIR="'+join_paths(meson.source_root(), 'src', 'lib')+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eolian', eolian_suite,
  timeout: 30,
  env : test_env
)
