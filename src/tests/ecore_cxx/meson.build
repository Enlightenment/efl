ecore_cxx_suite_deps = [check, ecore_cxx]

ecore_cxx_suite_src = [
  'ecore_cxx_suite.cc',
  'ecore_cxx_suite.h',
  'ecore_cxx_test_safe_call.cc',
]


ecore_cxx_suite = executable('ecore_cxx_suite',
  ecore_cxx_suite_src,
  dependencies: [ecore_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

executable('ecore_cxx_suite_compile_test',
  'cxx_compile_test.cc',
  dependencies: [ecore_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('ecore_cxx-suite', ecore_cxx_suite,
  env : test_env
)
