foreach loader_inst : evas_image_loaders_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    if loader_type == 'shared'
      config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
      file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'image_loaders', loader, 'evas_image_load_'+loader+'.c')
      evas_package_modules = join_paths(dir_lib, 'evas', 'modules')
      mod_install_dir = join_paths(evas_package_modules, 'image_loaders', loader, version_name)
      shared_module('shared_loader_'+loader, file,
        c_args : package_c_args,
        include_directories : config_dir,
        dependencies : [eina, evas, eet, evas_ext_none_static_deps] + loader_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'libshared_loader_'+loader + '.' + sys_mod_extension)
    endif
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach
