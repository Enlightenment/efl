eldbus_cxx_suite_deps = [check, eldbus_cxx]

eldbus_cxx_suite_src = [
  'eldbus_cxx_suite.cc',
  'eldbus_cxx_suite.h',
  'eldbus_cxx_test_eldbus_client.cc',
  'eldbus_cxx_test_eldbus_connect.cc',
]

eldbus_cxx_suite = executable('eldbus_cxx_suite',
  eldbus_cxx_suite_src,
  dependencies: [eldbus_cxx_suite_deps, check],
  cpp_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eldbus_cxx-suite', eldbus_cxx_suite,
  env : test_env
)
