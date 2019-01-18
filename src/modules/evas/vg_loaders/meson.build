evas_vg_loaders_file = ['eet', 'svg']

foreach loader : evas_vg_loaders_file
	file = join_paths(loader, 'evas_vg_load_'+loader+'.c')
	static_library('vg_loader_'+loader, file,
		include_directories : config_dir,
		dependencies : evas_pre 
	)
  evas_static_list += [declare_dependency(
    sources: file,
  )]
  config_h.set('EVAS_STATIC_BUILD_VG_'+loader.to_upper(), '1')
endforeach
