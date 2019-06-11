
cmake_src = [
  'Ecore',
  'EcoreCxx',
  'Edje',
  'Eet',
  'EetCxx',
  'Eeze',
  'Efl',
  'Efreet',
  'Eina',
  'EinaCxx',
  'Eio',
  'Eldbus',
  'Elementary',
  'Elua',
  'Emile',
  'Emotion',
  'Eo',
  'EoCxx',
  'Eolian',
  'EolianCxx',
  'EthumbClient',
  'Ethumb',
  'Evas',
  'EvasCxx',
]

cmake_version_src = [
  'Efl',
  'Elementary',
]

conf = configuration_data()
conf.set('VERSION', meson.project_version())

foreach cmake_file : cmake_src
  configure_file(
    input: cmake_file+'Config.cmake.in',
    output: cmake_file+'Config.cmake',
    install_dir: join_paths(dir_lib, 'cmake', cmake_file),
    copy: true,
  )
  if cmake_version_src.contains(cmake_src)
    configure_file(
      input: cmake_version_file+'ConfigVersion.cmake.in',
      output: cmake_version_file+'ConfigVersion.cmake',
      install_dir: join_paths(dir_lib, 'cmake', cmake_version_file),
      configuration : conf,
    )
  else
    configure_file(
      input: 'EflConfigVersion.cmake.in',
      output: cmake_file+'ConfigVersion.cmake',
      install_dir: join_paths(dir_lib, 'cmake', cmake_file),
      configuration : conf,
    )
  endif
endforeach

configure_file(
  input: 'EolianHelper.cmake.in',
  output: 'EolianHelper.cmake',
  install_dir: join_paths(dir_lib, 'cmake', 'Eolian'),
  copy: true
)
