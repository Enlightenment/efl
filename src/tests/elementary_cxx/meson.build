elementary_cxx_suite_deps = [check, elementary_cxx]

executable('elementary_cxx_suite_compile_test',
  ['cxx_compile_test.cc', 'cxx_dummy_compile_test.cc'],
  dependencies: [elementary_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)
