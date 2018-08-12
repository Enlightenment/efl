efreet_suite_src = [
  'efreet_suite.c',
  'efreet_suite.h',
  'efreet_test_efreet.c',
  'efreet_test_efreet_cache.c'
]

efreet_suite = executable('efreet_suite',
  efreet_suite_src,
  dependencies: [check, efreet],
  include_directories : config_dir,
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('efreet-suite', efreet_suite,
  env : test_env
)

efreet_test_src = [
  'ef_data_dirs.c',
  'ef_desktop.c',
  'ef_icon_theme.c',
  'ef_ini.c',
  'ef_locale.c',
  'ef_menu.c',
  'ef_mime.c',
  'ef_test.h',
  'ef_utils.c',
  'main.c'
]

efreet_test = executable('efreet_test',
  efreet_test_src,
  dependencies: [check, efreet, ecore, ecore_file, efreet_mime],
  include_directories : config_dir,
  c_args : [
  package_c_args,
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"',
  '-DDATA_DIR="'+dir_data+'"']
)

additional_tests = [
  'efreet_async_test',
  'efreet_spec_test',
  'efreet_icon_cache_dump',
  'efreet_user_dir',
  join_paths('compare', 'efreet_alloc'),
  join_paths('compare', 'efreet_menu_alloc')
]

foreach additional_test : additional_tests
  efreet_tmp = executable(additional_test.underscorify(),
    additional_test+'.c',
    dependencies: [check, efreet, ecore, eet, ecore_file, efreet_mime],
    include_directories : config_dir,
    c_args : [
    package_c_args,
    '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
    '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"',
    '-DDATA_DIR="'+dir_data+'"']
  )

endforeach
