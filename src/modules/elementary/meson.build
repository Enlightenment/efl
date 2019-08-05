mods = [
  'access_output',
  'prefs',
  'test_entry',
  'test_map',
  join_paths('web', 'none')
]

foreach mod : mods
  mod_install_dir = join_paths(dir_lib, 'elementary', 'modules', mod, version_name)
  subdir(mod)
  module_files += join_paths(mod_install_dir, 'lib'+mod+'.'+sys_mod_extension)
endforeach
