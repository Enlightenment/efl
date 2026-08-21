priv_eo_files = [
   'generated_future.eo'
]

eolian_test_src = [
'eolian_parsing.c',
'eolian_aux.c',
'eolian_generation.c',
'eolian_generated_future.c',
'eolian_suite.c',
'eolian_suite.h',
'eolian_static.c'
]

priv_eo_file_target = []
foreach eo_file : priv_eo_files
  # Both generated files are declared, so ninja knows the .eo.c exists and puts
  # it back if it goes missing. Only the header goes into the target list: the
  # .eo.c is #included by a hand-written source, and meson would otherwise
  # compile it alone, where it cannot see the types it needs. Indexing keeps
  # the dependency on the whole target.
  _eo_ct = custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h', eo_file + '.c'],
    depfile : eo_file + '.d',
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'D:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-e', 'EOTEST_API',
                          '-gchD', '@INPUT@'])
  priv_eo_file_target += _eo_ct[0]
endforeach

eolian_suite = executable('eolian_suite',
  eolian_test_src, priv_eo_file_target,
  dependencies: [eolian, check, eo],
  c_args : [
  '-DEOLIAN_GEN="'+eolian_gen_path+'"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DEO_SRC_DIR="'+join_paths(meson.project_source_root(), 'src', 'lib')+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eolian', eolian_suite,
  env : test_env
)
