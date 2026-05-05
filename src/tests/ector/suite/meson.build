ector_suite_src = [
  'ector_suite.c',
  'ector_suite.h',
  'ector_test_init.c',
  'ector_test_span_collector.c',
  'ector_test_grad_atlas.c',
]

# Include directories for the test suite:
#   '..'           — resolves ../efl_check.h via the -I../src/tests/ector
#                    search path that meson adds automatically
#   gl_generic     — provides evas_ector_gl_span.h
#   freetype       — provides sw_ft_raster.h (needed by evas_ector_gl_span.h)
ector_suite_inc = [
  include_directories('..'),
  include_directories(
    join_paths('..', '..', '..', 'modules', 'evas', 'engines', 'gl_generic')),
  include_directories(
    join_paths('..', '..', '..', 'static_libs', 'freetype')),
]

# The span-collector and grad-atlas implementations are compiled directly into
# the test binary.  SPAN_COLLECTOR_TEST_BUILD disables the
# ector_software_private.h include so that the test does not need the full EFL
# generated-header tree.  SPAN_GRAD_ATLAS_TEST_BUILD enables the test-only
# skip-GL flag so the atlas lookup logic runs without a real GL context.
ector_suite_span_src = [
  join_paths('..', '..', '..', 'modules', 'evas', 'engines',
             'gl_generic', 'evas_ector_gl_span.c'),
  join_paths('..', '..', '..', 'modules', 'evas', 'engines',
             'gl_generic', 'evas_ector_gl_grad_atlas.c'),
]

ector_suite = executable('ector_suite',
  ector_suite_src + ector_suite_span_src,
  include_directories : ector_suite_inc,
  dependencies: [eo, ector, check],
  c_args : [
    '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
    '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"',
    '-DSPAN_COLLECTOR_TEST_BUILD=1',
    '-DSPAN_GRAD_ATLAS_TEST_BUILD=1',
  ]
)

test('ector-suite', ector_suite,
  env : test_env
)
