evas_model_savers_file = ['eet', 'obj', 'ply']

foreach loader : evas_model_savers_file
	file = join_paths(loader, 'evas_model_save_'+loader+'.c')

	tmp = static_library('model_saver_'+loader, file,
		include_directories : config_dir,
		dependencies : evas_pre
	)

  evas_static_list += [declare_dependency(
    include_directories: include_directories('.'),
    sources: file,
    dependencies: evas_pre,
  )]
endforeach
