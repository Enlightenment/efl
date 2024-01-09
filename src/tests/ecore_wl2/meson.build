ecore_wl2_suite_src = [
  'ecore_wl2_suite.c',
  'ecore_wl2_suite.h',
  'ecore_wl2_tests_helpers.h',
  'ecore_wl2_test_ecore_wl2.c',
  'ecore_wl2_test_display.c',
  'ecore_wl2_test_window.c',
  'ecore_wl2_test_input.c',
  'ecore_wl2_test_output.c'
]

wl2_test_gl_deps = []

if get_option('opengl') == 'es-egl'
    ecore_wl2_suite_src += 'ecore_wl2_tests_helper_egl.h'
    wl2_test_gl_deps += dependency('egl')
    wl2_test_gl_deps += dependency('gl')
endif

ecore_wl2_suite = executable('ecore_wl2_suite',
  ecore_wl2_suite_src,
  dependencies: [ecore_wl2, ecore, ecore_input, check, wayland_protocol, wayland_client, dependency('wayland-egl'), wl2_test_gl_deps],

  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('ecore_wl2-suite', ecore_wl2_suite,
  env : test_env,
  timeout : master_timeout
)
