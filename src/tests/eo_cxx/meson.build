eo_cxx_suite_deps = [check, eina_cxx, eo_cxx]

eo_cxx_suite_src = [
  'eo_cxx_suite.cc',
  'eo_cxx_suite.h'
]

eo_cxx_suite = executable('eo_cxx_suite',
  eo_cxx_suite_src,
  include_directories : config_dir,
  dependencies: [eo_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo_cxx-suite', eo_cxx_suite,
  env : test_env
)
