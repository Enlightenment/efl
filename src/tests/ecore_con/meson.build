ecore_con_suite_src = [
  'ecore_con_suite.c',
  'ecore_con_test_ecore_con.c',
  'ecore_con_test_ecore_con_url.c',
  'ecore_con_test_ecore_con_eet.c',
  'ecore_con_test_efl_net_ip_address.c',
  'ecore_con_suite.h'
]

ecore_con_suite = executable('ecore_con_suite',
  ecore_con_suite_src,
  dependencies: [ecore_con, eet, ecore, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('ecore_con-suite', ecore_con_suite,
	is_parallel: true,
)