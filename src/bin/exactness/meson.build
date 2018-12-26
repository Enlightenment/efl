exactness_bin = executable('exactness',
  [ 'exactness.c' ],
  dependencies: [ ecore, ecore_evas, ecore_file, exactness ],
  install: true,
  )

exactness_inject_bin = executable('exactness_inject',
  [ 'injector.c' ],
  dependencies: [ elementary, exactness ],
  install: true,
  )

exactness_inspect_bin = executable('exactness_inspect',
  [ 'inspect.c' ],
  dependencies: [ elementary, exactness ],
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
  depends : edje_cc)

exactness_play_bin = executable('exactness_play',
  [ 'player.c', edjs ],
  dependencies: [ elementary, exactness ],
  c_args: '-DDATA_DIR="'+join_paths(dir_data, 'exactness')+'"',
  install: true,
  )

exactness_record_bin = executable('exactness_record',
  [ 'recorder.c' ],
  dependencies: [ elementary, exactness ],
  install: true,
  )

