mods = ['ibus', 'xim']

if dependency('scim-1.0', required : false).found()
  mods += ['scim']
endif

if get_option('ecore_wl2')
  mods += ['wayland']
endif


foreach mod_name : mods
  mod_src = []
  mod_deps = []
  subdir(mod_name)
  config_h.set('BUILD_ECORE_IMF_'+mod_name.to_upper(), '1')

  mod_install_dir =  join_paths(dir_package_modules, mod_name, module_arch)

  shared_module(mod_name,
      mod_src,
      dependencies: mod_deps,
      install: true,
      install_dir : mod_install_dir
  )
  module_files += join_paths(mod_install_dir, 'lib'+mod_name+'.so')

endforeach
