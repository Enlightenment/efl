checkme_files = ['ecore', 'ecore_imf', 'ecore_x', 'eeze', 'efreet', 'elua', 'emotion', 'ethumb', 'ethumb_client', 'evas']
foreach checkme : checkme_files
  install_data(join_paths(checkme, 'checkme'),
    install_dir : join_paths(dir_data, checkme)
  )
endforeach

subdir('eo')
subdir('edje')
subdir('embryo')
subdir(join_paths('ethumb', 'frames'))
subdir('elementary')
