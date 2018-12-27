eo_debug_config = configuration_data()

eo_debug_config.set('prefix', dir_prefix)
eo_debug_config.set('exec_prefix', '${prefix}')
eo_debug_config.set('libdir', dir_lib)

configure_file(
  input: 'eo_debug.in',
  output: 'eo_debug',
  configuration: eo_debug_config,
  install_dir: dir_bin)

# make sure it's executable
o = join_paths(dir_bin, 'eo_debug')
meson.add_install_script(chmod, 'a+r+x', o)
