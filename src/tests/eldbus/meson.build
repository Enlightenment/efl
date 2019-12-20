eldbus_suite_src = [
  'eldbus_suite.c',
  'eldbus_test_eldbus_init.c',
  'eldbus_test_eldbus_model.h',
  'eldbus_fake_server.h',
  'eldbus_test_eldbus_model.c',
  'eldbus_test_eldbus_model_connection.c',
  'eldbus_test_eldbus_model_object.c',
  'eldbus_test_eldbus_model_proxy.c',
  'eldbus_test_eldbus_model_method.c',
  'eldbus_test_eldbus_model_signal.c',
  'eldbus_test_eldbus_object.c',
  'eldbus_test_eldbus_message.c',
  'eldbus_test_eldbus_signal_handler.c',
  'eldbus_test_eldbus_proxy.c',
  'eldbus_test_eldbus_pending_cancel.c',
  'eldbus_test_fake_server_eldbus_model_proxy.c',
  'eldbus_fake_server.c',
  'eldbus_suite.h'
]

eldbus_suite = executable('eldbus_suite',
  eldbus_suite_src,
  dependencies: [eldbus, ecore, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eldbus-suite', eldbus_suite,
  env : test_env,
)
