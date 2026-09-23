# The slave is driven directly over its pipes, so the test needs to know
# where it was built. It belongs to ethumb_client, which is configured before
# this, so the variable is in scope -- unless that package is disabled, and
# then there is nothing to test.
if is_variable('ethumbd_slave')
  ethumb_suite_src = [
    'ethumb_suite.c',
    'ethumb_suite.h',
    'ethumb_test_slave.c',
  ]

  ethumb_suite = executable('ethumb_suite',
    ethumb_suite_src,
    dependencies: [eina, ecore, ethumb, check],
    c_args : [
      '-DETHUMBD_SLAVE_PATH="'+ethumbd_slave.full_path()+'"',
      '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
      '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"',
      '-DNEED_RUN_IN_TREE=1',
    ])

  test('ethumb-suite', ethumb_suite,
    env : test_env,
    timeout : master_timeout
  )
endif
