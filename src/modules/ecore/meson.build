system_modules = [
  'systemd',
  'upower',
  #'tizen',
]

foreach system_module : system_modules
  mod_install_dir = join_paths(dir_lib, 'ecore', 'system', system_module, version_name)
  subdir(join_paths('system', system_module))
endforeach
