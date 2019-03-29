efl_ui_suite_behavior_src = [
  join_paths('..','suite_helpers.c'),
  join_paths('..','suite_helpers.h'),
  join_paths('..','elm_test_init.c'),
  'efl_ui_spec_suite.c',
  'efl_test_pack.c',
  'efl_test_pack_linear.c',
  'efl_test_container.c',
]

test_generator = find_program('generator.py')

generated_test_parts = custom_target('generate_test_suite',
  output: 'efl_ui_spec_suite_gen.x',
  command: [test_generator, '@OUTPUT@'],
)

efl_ui_behavior_suite = executable('efl_ui_spec_suite',
  efl_ui_suite_behavior_src + [generated_test_parts],
  dependencies: [check, eina, elementary, elementary_deps],
  include_directories : [config_dir] + [elementary_config_dir] + [include_directories('../')],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"',
  '-DELM_IMAGE_DATA_DIR="'+join_paths(meson.source_root(), 'data', 'elementary')+'"',
  '-DELM_TEST_DATA_DIR="'+join_paths(meson.build_root(), 'data', 'elementary')+'"',
  ]
)

test('efl_ui_spec-suite', efl_ui_behavior_suite,
  env : test_env
)
