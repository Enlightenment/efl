eo_suite_src = [
  'eo_test_class_simple.c',
  'eo_test_class_simple.h',
  'eo_test_class_singleton.c',
  'eo_test_class_singleton.h',
  'eo_test_domain.c',
  'eo_test_domain.h',
  'eo_suite.c',
  'eo_suite.h',
  'eo_error_msgs.h',
  'eo_error_msgs.c',
  'eo_test_class_errors.c',
  'eo_test_class_behaviour_errors.c',
  'eo_test_call_errors.c',
  'eo_test_general.c',
  'eo_test_value.c',
  'eo_test_event.c',
  'eo_test_threaded_calls.c',
  'eo_test_init.c',
  'eo_test_lifecycle.c',
  'eo_test_reflection.c'
]

eo_suite = executable('eo_suite',
  eo_suite_src,
  dependencies: [eo, check, valgrind],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-suite', eo_suite,
  env : test_env,
  timeout : master_timeout
)

eo_suite = executable('eo_suite_dbg',
  eo_suite_src,
  dependencies: [eo_dbg, check, valgrind],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DEO_DEBUG=1',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-suite-dbg', eo_suite,
  env : test_env,
  timeout : master_timeout
)

eo_suite = executable('eo_suite_fallback',
  eo_suite_src,
  dependencies: [eo_dbg, check, valgrind],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DEO_DEBUG=1',
  '-D_EO_ADD_FALLBACK_FORCE=1',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-suite-fallback', eo_suite,
  env : test_env,
  timeout : master_timeout
)
