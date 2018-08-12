elput_suite_src = [
  'elput_suite.c',
  'elput_suite.h',
  'elput_test_elput.c'
]

elput_suite = executable('elput_suite',
  elput_suite_src,
  dependencies: [elput, ecore, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('elput-suite', elput_suite,
  is_parallel: true,
)