subdir('data')

emotion_eo_suite = executable('emotion_eo_suite',
  'emotion_test_main-eo.c', themes,
  dependencies: [check, eo, evas, emotion, ecore_evas, edje, ecore],
  include_directories : config_dir,
  c_args : [
  '-DPACKAGE_DATA_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

emotion_suite = executable('emotion_suite',
  'emotion_test_main.c', themes,
  dependencies: [check, eo, evas, emotion, ecore_evas, edje, ecore],
  include_directories : config_dir,
  c_args : [
  '-DPACKAGE_DATA_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)
