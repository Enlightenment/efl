exactness_bin = executable('exactness',
  [ 'exactness.c', 'common.c', 'common.h' ],
  dependencies: [ ecore, ecore_evas, ecore_file, elementary],
  c_args: '-DDATA_DIR="'+join_paths(dir_data, 'exactness')+'"',
  install: true,
  )

exactness_inject_bin = executable('exactness_inject',
  [ 'injector.c', 'common.c', 'common.h' ],
  dependencies: [ elementary ],
  c_args: '-DDATA_DIR="'+join_paths(dir_data, 'exactness')+'"',
  install: true,
  )

exactness_inspect_bin = executable('exactness_inspect',
  [ 'inspect.c', 'common.c', 'common.h' ],
  dependencies: [ elementary ],
  c_args: '-DDATA_DIR="'+join_paths(dir_data, 'exactness')+'"',
  install: true,
  )

edjs = custom_target('player_entry',
  input : 'player_entry.edc',
  output : 'player_entry.edj',
  install : true,
  install_dir : 'share/exactness',
  command : edje_cc_exe + [
    '-id', join_paths(meson.source_root(), 'data', 'elementary', 'themes', 'img'),
    '-sd', join_paths(meson.source_root(), 'data', 'elementary', 'themes', 'snd'),
    '@INPUT@', '@OUTPUT@'],
  depends : edje_depends)

exactness_play_bin = library('exactness_play',
  [ 'player.c', 'common.c', 'common.h', edjs ],
  dependencies: [ elementary, dl],
  c_args: '-DDATA_DIR="'+join_paths(dir_data, 'exactness')+'"',
  install: true,
  version : meson.project_version()
)

exactness_record_bin = library('exactness_record',
  [ 'recorder.c', 'common.c', 'common.h' ],
  dependencies: [ elementary, dl],
  c_args: '-DDATA_DIR="'+join_paths(dir_data, 'exactness')+'"',
  install: true,
  version : meson.project_version()
)

exactness_env = configuration_data()
exactness_env.set_quoted('EXACTNESS_PLAY_PRELOAD_PATH', exactness_play_bin.full_path())
exactness_env.set_quoted('EXACTNESS_RECORD_PRELOAD_PATH', exactness_record_bin.full_path())
exactness_env.set_quoted('VERSION', meson.project_version())

configure_file(
  input: 'exactness_record.in',
  output: 'exactness_record',
  configuration: exactness_env,
  install_dir: get_option('bindir'),
  install_mode: 'rwxr-xr-x')

configure_file(
  input: 'exactness_play.in',
  output: 'exactness_play',
  configuration: exactness_env,
  install_dir: get_option('bindir'),
  install_mode: 'rwxr-xr-x')
