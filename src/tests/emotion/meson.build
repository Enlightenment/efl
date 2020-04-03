emotion_suite_bin = executable('emotion_suite',
  'emotion_suite.c', themes,
  dependencies: [eo, evas, emotion, ecore_evas, edje, ecore, check],
  include_directories : config_dir,
  install: true,
  c_args : [
  '-DPACKAGE_DATA_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('emotion-suite', emotion_suite_bin,
  env : test_env
)
