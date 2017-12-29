
evas_image_savers_file = [
['eet', [eet]], 
['jpeg', [jpeg]], 
['png', [png]], 
['tgv', [rg_etc, lz4]], 
['tiff', [tiff]], 
['webp', []]
]

foreach loader_inst : evas_image_savers_file
  loader = loader_inst[0]
  loader_deps = loader_inst[1]

  if get_option('evas-loaders-disabler').contains(loader) == false
    file = join_paths(loader, 'evas_image_save_'+loader+'.c')

    tmp = static_library('image_saver_'+loader, file,
      include_directories : config_dir,
      dependencies : [evas_pre] + loader_deps
    )

    evas_static_list += [declare_dependency(
      sources: file,
      dependencies: loader_deps,
    )]

    config_h.set('EVAS_STATIC_BUILD_'+loader.to_upper(), '1')
  else
    message('Image saver '+loader+' disabled')
  endif
endforeach
