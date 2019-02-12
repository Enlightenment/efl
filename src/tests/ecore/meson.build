
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
  'ecore_test_pipe.c',
  'ecore_suite.h'
]

ecore_suite_deps = [m]
ecore_suite_deps += ecore
ecore_suite_deps += ecore_file
ecore_suite_deps += ecore_evas
ecore_suite_deps += ecore_input
ecore_suite_deps += ecore_imf

test_eo_files = [
  'ecore_audio_out_test.eo',
]
test_eo_file_target = []

foreach eo_file : test_eo_files
    test_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : false,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

if get_option('audio')
  ecore_suite_deps += ecore_audio
  ecore_suite_src += 'ecore_test_ecore_audio.c'
  ecore_suite_src += test_eo_file_target
endif

if get_option('fb')
  ecore_suite_deps += ecore_fb
  ecore_suite_src += 'ecore_test_ecore_fb.c'
endif

if get_option('drm')
  ecore_suite_deps += ecore_drm2
endif

if get_option('x11')
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

efl_app_suite_src = [
  'efl_app_suite.c',
  'efl_app_suite.h',
  'efl_app_test_loop.c',
  'efl_app_test_loop_fd.c',
  'efl_app_test_loop_timer.c',
  'efl_app_test_promise.c'
]

efl_app_suite_deps = [m]
efl_app_suite_deps += ecore

efl_app_suite = executable('efl_app_suite',
  efl_app_suite_src,
  dependencies: [efl_app_suite_deps, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('ecore-suite', ecore_suite,
  timeout : 10*60,
  env : test_env
)

test('efl-app', efl_app_suite,
  timeout : 10*60,
  env : test_env
)
