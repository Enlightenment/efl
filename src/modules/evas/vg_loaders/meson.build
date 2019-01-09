evas_vg_loaders_file = [
  ['eet', [eet]],
  ['json', [json]],
  ['svg', []],
]

foreach loader_inst : evas_vg_loaders_file
	loader = loader_inst[0]
	loader_deps = loader_inst[1]

	if (get_option('evas-loaders-disabler').contains(loader) == false)
		file = join_paths(loader, 'evas_vg_load_'+loader+'.c')

		static_library('vg_loader_'+loader, file,
			include_directories : config_dir,
			dependencies : [evas_pre] + loader_deps
		)

		evas_static_list += [declare_dependency(
			sources: file,
			dependencies: loader_deps,
		)]

		config_h.set('EVAS_STATIC_BUILD_VG_'+loader.to_upper(), '1')
	endif
endforeach
