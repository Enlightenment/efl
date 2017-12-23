
ecore_suite_src = [
  'ecore_suite.c',
  'ecore_test_ecore.c',
  'ecore_test_ecore_imf.c',
  'ecore_test_timer.c',
  'ecore_test_ecore_evas.c',
  'ecore_test_animator.c',
  'ecore_test_ecore_thread_eina_thread_queue.c',
  'ecore_test_ecore_input.c',
  'ecore_test_ecore_file.c',
  'ecore_test_promise.c',
  'ecore_test_promise2.c',
  'ecore_test_job.c',
  'ecore_test_args.c',
  'ecore_suite.h'
]

ecore_suite = executable('ecore_suite',
  ecore_suite_src,
  dependencies: [ecore, ecore_imf, ecore_evas, ecore_input, ecore_file, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('ecore-suite', ecore_suite, 
  is_parallel: true,
  timeout : 10*60,
)