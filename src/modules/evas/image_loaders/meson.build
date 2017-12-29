evas_image_loaders_file = [
  ['bmp', []],
  ['eet', [eet]],
  ['generic', [rt]],
  ['gif', [giflib]],
  ['ico', []],
  ['jpeg', [jpeg]],
  ['pmaps', []],
  ['png', [png]],
  ['psd', []],
  ['tga', []],
  ['tgv', [rg_etc, lz4]],
  ['tiff', [tiff]],
  ['wbmp', []],
  ['webp', [webp]],
  ['xpm', []],
]

foreach loader_inst : evas_image_loaders_file
	loader = loader_inst[0]
	loader_deps = loader_inst[1]


  if get_option('evas-loaders-disabler').contains(loader) == false
  	file = join_paths(loader, 'evas_image_load_'+loader+'.c')

  	tmp = static_library('image_loader_'+loader, file,
  		include_directories : config_dir,
  		dependencies : [evas_pre] + loader_deps
  	)

    evas_static_list += [declare_dependency(
      sources: file,
      dependencies: loader_deps,
    )]

  	config_h.set('EVAS_STATIC_BUILD_'+loader.to_upper(), '1')
    config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach