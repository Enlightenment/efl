ecore_wl2_engines = [
  'dmabuf'
]

foreach engine : ecore_wl2_engines
   mod_install_dir = join_paths(dir_lib, 'ecore_wl2', 'engines', engine, version_name)
   subdir(join_paths('engines', engine))
   module_files += join_paths(mod_install_dir, 'lib'+engine+'.'+sys_mod_extension)
endforeach
