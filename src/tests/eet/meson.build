eet_suite_src = [
  'eet_test_cache.c',
  'eet_test_cipher.c',
  'eet_test_common.c',
  'eet_test_connection.c',
  'eet_test_data.c',
  'eet_test_file.c',
  'eet_test_identity.c',
  'eet_test_image.c',
  'eet_test_init.c',
  'eet_suite.c',
  'eet_test_common.h',
  'eet_suite.h'
]

eet_suite = executable('eet_suite',
  eet_suite_src,
  dependencies: [eet, check],
  c_args : [
  '-DTESTS_WD="`pwd`"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eet-suite', eet_suite,
  is_parallel: true,
)
