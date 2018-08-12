emile_suite_src = [
  'emile_suite.c',
  'emile_suite.h',
  'emile_test_base.c',
  'emile_test_base64.c'
]

emile_suite = executable('emile_suite',
  emile_suite_src,
  dependencies: [emile, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('emile-suite', emile_suite,
  env : test_env
)
