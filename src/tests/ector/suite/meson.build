ector_suite_src = [
  'ector_suite.c',
  'ector_suite.h',
  'ector_test_init.c',
]

ector_suite = executable('ector_suite',
  ector_suite_src,
  include_directories : include_directories('..'),
  dependencies: [eo, ector, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('ector-suite', ector_suite,
  env : test_env
)
