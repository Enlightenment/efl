mods = []

if get_option('ecore_x')
  mods +=['ibus', 'xim', 'scim']
endif

if get_option('ecore_wl2')
  mods += ['wayland']
endif


loader_disabler = get_option('ecore-imf-loaders-disabler')

foreach mod_name : mods
  mod_src = []
  mod_deps = []

  if loader_disabler.contains(mod_name) == false
    mod_install_dir =  join_paths(dir_package_modules, mod_name, module_arch)
    subdir(mod_name)
    module_files += join_paths(mod_install_dir, 'lib'+mod_name+'.'+sys_lib_extension)
    config_h.set('BUILD_ECORE_IMF_'+mod_name.to_upper(), '1')
  endif
endforeach
