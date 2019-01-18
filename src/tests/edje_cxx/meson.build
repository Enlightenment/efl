edje_cxx_suite_deps = [check, edje_cxx]

executable('edje_cxx_suite_compile_test',
  'cxx_compile_test.cc',
  dependencies: [edje_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)
