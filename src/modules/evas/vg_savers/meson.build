evas_vg_savers_file = ['eet', 'svg']

foreach loader : evas_vg_savers_file
	file = join_paths(loader, 'evas_vg_save_'+loader+'.c')
	tmp = static_library('vg_saver_'+loader, file,
		include_directories : config_dir,
    dependencies: [eo, ector, emile, freetype, eet, evas_pre],
	)
  evas_static_list += [declare_dependency(
    sources: file,
    dependencies: [eo, ector, emile, freetype, eet, evas_pre],
  )]
  config_h.set('EVAS_STATIC_BUILD_VG_'+loader.to_upper(), '1')
endforeach