subdir('data')

emotion_test_eo = executable('emotion_test-eo',
  'emotion_test_main-eo.c', themes,
  dependencies: [eo, evas, emotion, ecore_evas, edje, ecore],
  include_directories : config_dir,
  install: true,
  c_args : [
  '-DPACKAGE_DATA_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

emotion_test = executable('emotion_test',
  'emotion_test_main.c', themes,
  dependencies: [eo, evas, emotion, ecore_evas, edje, ecore],
  include_directories : config_dir,
  install: true,
  c_args : [
  '-DPACKAGE_DATA_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)
