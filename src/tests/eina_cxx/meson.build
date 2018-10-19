eina_cxx_suite_deps = [check]

eina_cxx_suite_src = [
  'eina_cxx_suite.cc',
  'eina_cxx_test_inlist.cc',
  'eina_cxx_test_log.cc',
  'eina_cxx_test_inarray.cc',
  'eina_cxx_test_iterator.cc',
  'eina_cxx_test_ptrarray.cc',
  'eina_cxx_test_ptrlist.cc',
  'eina_cxx_test_stringshare.cc',
  'eina_cxx_test_error.cc',
  'eina_cxx_test_accessor.cc',
  'eina_cxx_test_thread.cc',
  'eina_cxx_test_optional.cc',
  'eina_cxx_test_value.cc',
  'simple.c',
  'eina_cxx_suite.h'
]


pub_eo_files = [
  'simple.eo'
]

foreach eo_file : pub_eo_files
  priv_eo_file_target += custom_target('eolian_gen_eina_cxx_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-gch', '@INPUT@'])
  priv_eo_file_target += custom_target('eolian_cxx_gen_eina_cxx_' + eo_file,
      input : eo_file,
      output : [eo_file + '.hh'],
      command : [eolian_cxx_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                                 '-o', join_paths(meson.current_build_dir(), eo_file + '.hh'),
                                 '@INPUT@'])
endforeach

eina_cxx_suite = executable('eina_cxx_suite',
  eina_cxx_suite_src + priv_eo_file_target,
  include_directories : config_dir,
  dependencies: [eina_cxx_suite_deps, check, eina, eina_cxx, eo_cxx, ecore],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"'
  ]
)

test('eina_cxx-suite', eina_cxx_suite,
  timeout : 10*60,
  env : test_env
)
