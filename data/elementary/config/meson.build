configs = ['default', 'mobile', 'standard']

foreach config : configs
  config_file = join_paths(meson.current_source_dir(), config, 'base.src.in')
  config_icon = join_paths(meson.current_source_dir(), config, 'icon.png')
  config_desktop = join_paths(meson.current_source_dir(), config, 'profile.desktop')

  install_data([config_icon, config_desktop],
    install_dir : join_paths(dir_data, 'elementary', 'config', config)
  )

  subdir(config)
endforeach

custom_target('config_profile',
  input: 'profile.src',
  output: 'profile.cfg',
  command: [eet_bin, '-i', '@OUTPUT@', 'config', '@INPUT@', '0'],
  install: true,
  install_dir: join_paths(dir_data, 'elementary', 'config'),
)
