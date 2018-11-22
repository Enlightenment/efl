desktop_files = files(
  'elementary_config.desktop',
  'elementary_test.desktop',
  'elementary_perf.desktop')
elementary_icon = files('elementary.png')

install_data(desktop_files,
  install_dir : join_paths(dir_data, 'applications')
)

install_data(elementary_icon,
  install_dir : join_paths(dir_data, 'icons', 'hicolor', '128x128', 'apps')
)
