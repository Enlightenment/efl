  custom_target('config_'+config,
    input: config_file,
    output: 'base.cfg',
    command: [eet_bin, '-e', '@OUTPUT@', 'config', '@INPUT@', '0'],
    install: true,
    install_dir: join_paths(dir_data, 'elementary', 'config', config),
  )
