evas_suite_src = [
  'evas_suite.c',
  'evas_test_new.c',
  'evas_test_init.c',
  'evas_test_object.c',
  'evas_test_object_smart.c',
  'evas_test_textblock.c',
  'evas_buffer_helper.c',
  'evas_test_text.c',
  'evas_test_callbacks.c',
  'evas_test_render_engines.c',
  'evas_test_filters.c',
  'evas_test_image.c',
  'evas_test_mask.c',
  'evas_test_evasgl.c',
  'evas_test_focus.c',
  'evas_test_events.c',
  'evas_tests_helpers.h',
  'evas_suite.h',
  'efl_test_canvas.c',
  'efl_test_canvas2.c',
  'efl_test_canvas3.c',
  'efl_canvas_animation.c',
  'evas_test_map.c',
]

evas_suite = executable('evas_suite',
  evas_suite_src,
  dependencies: [evas_bin, evas, ecore_evas, dl, check, evas_ext_none_static_deps, eet], #external deps needed here since tests do include internal headers
  include_directories: include_directories(join_paths('..', '..', 'modules', 'evas', 'engines', 'buffer')),
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('evas-suite', evas_suite,
  env : test_env,
  timeout : master_timeout
)
