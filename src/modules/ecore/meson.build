
if sys_windows == false and sys_osx == false

  system_modules = [
    'systemd',
    'upower',
    #'tizen',
  ]

  foreach system_module : system_modules
    mod_install_dir = join_paths(dir_lib, 'ecore', 'system', system_module, version_name)
    subdir(join_paths('system', system_module))
  endforeach

endif
