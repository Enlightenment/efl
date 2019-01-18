eet_cxx_suite_deps = [check, eet_cxx]

eet_cxx_suite_src = [
  'eet_cxx_suite.cc',
  'eet_cxx_suite.h',
  'eet_cxx_test_descriptors.cc',
]

eet_cxx_suite = executable('eet_cxx_suite',
  eet_cxx_suite_src,
  dependencies: [eet_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)


test('ecore_cxx-suite', eet_cxx_suite,
  timeout : 10*60,
  env : test_env
)
