eolian_cxx_suite_deps = [check, eolian_cxx, eina_cxx, eo_cxx, ecore]

eolian_cxx_suite_src = files([
  'eolian_cxx_suite.cc',
  'eolian_cxx_test_parse.cc',
  'a.c',
  'b.c',
  'c.c',
  'd.c',
  'eolian_cxx_test_binding.cc',
  'eolian_cxx_test_address_of.cc',
  'eolian_cxx_test_wrapper.cc',
  'simple.c',
  'name_name.c',
  'name_name_cxx.cc',
  'generic.c',
  'name1_name2_type_generation.c',
  'eolian_cxx_test_inheritance.cc',
  'eolian_cxx_test_generate.cc',
  'eolian_cxx_test_documentation.cc',
  'eolian_cxx_test_cyclic.cc',
  'complex.c',
  'complex_cxx.cc',
  'eolian_cxx_suite.h'
])

pub_eo_files = [
  'a.eo',
  'b.eo',
  'c.eo',
  'd.eo',
  'complex.eo',
  'cyclic1.eo',
  'cyclic2.eo',
  'docs.eo',
  'generic.eo',
  'generic_interface.eo',
  'name1_name2_type_generation.eo',
  'name_name.eo',
  'ns_name.eo',
  'ns_name_other.eo',
  'simple.eo'
]

pub_eo_file_target = []

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_cxx_gen_' + eo_file,
      input : eo_file,
      output : [eo_file + '.hh'],
      command : [eolian_cxx_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                                 '-o', join_paths(meson.current_build_dir(), eo_file + '.hh'),
                                 '@INPUT@'])
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
      input : eo_file,
      output : [eo_file + '.h'],
      command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                             '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                             '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                             '-gch', '@INPUT@'])
endforeach

eolian_cxx_suite = executable('eolian_cxx_suite',
  eolian_cxx_suite_src + pub_eo_file_target,
  dependencies: [eolian_cxx_suite_deps, eo_cxx, check],
  # package_c_args contains -D definitions for the package
  cpp_args : package_c_args +[
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DEO_SRC_DIR="'+join_paths(meson.source_root(), 'src', 'lib')+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eolian_cxx-suite', eolian_cxx_suite,
  timeout : 10*60,
  env : test_env
)
