edje_externals = ['emotion', 'elementary']

foreach edje_external : edje_externals
  #!?%$§#! custom nane for edje externals
  if (edje_external == 'elementary')
    mod_install_dir = join_paths(dir_lib, 'edje', 'modules', 'elm', version_name)
  else
    mod_install_dir = join_paths(dir_lib, 'edje', 'modules', edje_external, version_name)
  endif
  subdir(edje_external)
endforeach

