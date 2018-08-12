generic_loaders = ['gst', 'pdf',
'ps',
'raw',
'svg',
'xcf']

generic_src = []
generic_deps = []

subdir('common')

common = static_library('evas_loader_common',
    generic_src,
    include_directories : config_dir,
    dependencies: [generic_deps, rt],
)

foreach loader : generic_loaders
  generic_src = []
  generic_deps = []
  generic_support = []

  if get_option('evas-loaders-disabler').contains(loader) == false
    subdir(loader)

    tmp = shared_module('image_loader.'+loader,
        generic_src,
        include_directories : config_dir + [include_directories('common')],
        link_with : common,
        dependencies: [eina, generic_deps],
        name_prefix : 'evas_',
        install_dir : join_paths(dir_lib, 'evas', 'utils'),
        install : true,
    )

    set_variable(loader, tmp)
    mod = join_paths(dir_lib, 'evas', 'utils', 'evas_image_loader.'+loader+'.so')
    evas_loader_map += [[mod, generic_support]]
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach
