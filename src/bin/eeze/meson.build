eeze_bins = ['disk_ls', 'mount', 'scanner', 'scanner_monitor', 'umount']

foreach eeze_bin : eeze_bins
	src_file += join_paths('eeze_'+eeze_bin, 'eeze_'+eeze_bin+'.c')
	tmp = executable('eeze_'+eeze_bin, src_file,
		dependencies: [eeze],
		install: true,
	)
  set_variable('eeze_'+eeze_bin, tmp)
endforeach