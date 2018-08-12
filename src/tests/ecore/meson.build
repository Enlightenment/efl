
ecore_suite_src = [
  'ecore_suite.c',
  'ecore_test_ecore.c',
  'ecore_test_ecore_imf.c',
  'ecore_test_idle.c',
  'ecore_test_timer.c',
  'ecore_test_ecore_evas.c',
  'ecore_test_animator.c',
  'ecore_test_ecore_thread_eina_thread_queue.c',
  'ecore_test_ecore_input.c',
  'ecore_test_ecore_file.c',
  'ecore_test_job.c',
  'ecore_test_args.c',
  'ecore_suite.h'
]

ecore_suite_deps = [m]
ecore_suite_deps += ecore
ecore_suite_deps += ecore_file
ecore_suite_deps += ecore_evas
ecore_suite_deps += ecore_input
ecore_suite_deps += ecore_imf

if get_option('ecore_audio')
  ecore_suite_deps += ecore_audio
  ecore_suite_src += 'ecore_test_ecore_audio.c'
endif

if get_option('ecore_fb')
  ecore_suite_deps += ecore_fb
  ecore_suite_src += 'ecore_test_ecore_fb.c'
endif

if get_option('ecore_drm2')
  ecore_suite_deps += ecore_drm2
endif

if get_option('ecore_x')
  ecore_suite_src += 'ecore_test_ecore_x.c'
  ecore_suite_deps += ecore_x
endif

ecore_suite = executable('ecore_suite',
  ecore_suite_src,
  dependencies: [ecore_suite_deps, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('ecore-suite', ecore_suite,
  is_parallel: true,
  timeout : 10*60,
)
