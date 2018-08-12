mod_install_dir = join_paths(dir_lib, 'ecore', 'system', module_arch)


if get_option('systemd')
  subdir(join_paths('system', 'systemd'))
endif
#disable for now
#subdir(join_paths('system', 'tizen'))
subdir(join_paths('system', 'upower'))
