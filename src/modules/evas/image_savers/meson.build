evas_image_savers_file = [
  evas_loader_saver_mod_eet,
  evas_loader_saver_mod_jpeg,
  evas_loader_saver_mod_png,
  evas_loader_saver_mod_tgv,
  evas_loader_saver_mod_tiff,
  evas_loader_saver_mod_webp
]

foreach loader_inst : evas_image_savers_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
    file = join_paths(loader, 'evas_image_save_'+loader+'.c')
    if loader_type == 'shared'
      evas_package_modules = join_paths(dir_lib, 'evas', 'modules')
      mod_install_dir = join_paths(evas_package_modules, 'image_savers', loader, version_name)
      shared_module(loader, file,
        include_directories : config_dir,
        dependencies : [evas_pre] + loader_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'lib'+loader + '.' + sys_mod_extension)
    else
      config_h.set('EVAS_STATIC_BUILD_'+loader.to_upper(), '1')
      tmp = static_library('image_saver_'+loader, file,
        include_directories : config_dir,
        dependencies : [evas_pre] + loader_deps
      )
      evas_static_list += [declare_dependency(
        sources: file,
        dependencies: loader_deps,
      )]
    endif
  else
    message('Image saver '+loader+' disabled')
  endif
endforeach
