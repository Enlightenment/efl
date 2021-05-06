custom_target('config_'+config,
  input: config_file,
  output: 'base.cfg',
  command: [eet_bin, '-e', '@OUTPUT@', 'config', '@INPUT@', '1'],
  install: true,
  install_dir: join_paths(dir_data, 'elementary', 'config', config),
)
