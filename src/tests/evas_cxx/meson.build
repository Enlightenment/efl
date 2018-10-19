evas_cxx_suite_deps = [check, evas_cxx]

executable('evas_cxx_suite_compile_test',
  'cxx_compile_test.cc',
  dependencies: [evas_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)
