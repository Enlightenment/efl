eeze_suite_src = [
  'eeze_suite.c',
  'eeze_test_init.c',
  'eeze_test_net.c',
  'eeze_test_sensor.c',
  'eeze_test_udev.c',
  'eeze_suite.h'
]

eeze_suite = executable('eeze_suite',
  eeze_suite_src,
  dependencies: [eeze, ecore, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eeze-suite', eeze_suite,
  env : test_env
)
