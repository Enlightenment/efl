eio_suite_src = [
  'eio_suite.c',
  'eio_model_test_file.c',
  'eio_model_test_monitor_add.c',
  'eio_test_monitor.c',
  'eio_test_sentry.c',
  'eio_test_file.c',
  'eio_test_manager.c',
  'eio_test_manager_xattr.c',
  'eio_test_xattr.c',
  'eio_test_common.c',
  'eio_test_common.h',
  'eio_test_map.c',
  'eio_test_eet.c',
  'eio_suite.h'
]

eio_suite = executable('eio_suite',
  eio_suite_src,
  include_directories : [config_dir],
  dependencies: [eo, ecore, eina, ecore_file, eio, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eio-suite', eio_suite,
  is_parallel: true,
  timeout : 10*60 #10 min. can be realistic, this testsuite is slow!
)