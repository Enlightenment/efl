install_data('eo_gdb.py',
  install_dir : join_paths(dir_data, 'eo', 'gdb')
)

configuration = configuration_data()
configuration.set('datadir', dir_data)

configure_file(
  input: 'libeo-gdb.py.in',
  output: 'libeo.so.'+meson.project_version()+'-gdb.py',
  configuration: configuration,
  install_dir: join_paths(dir_data, 'gdb', 'auto-load', 'usr', 'lib')
)
