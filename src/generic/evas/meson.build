generic_loaders = ['gst', 'pdf',
'ps',
'raw',
'rsvg',
'xcf']

generic_src = []
generic_deps = []

subdir('common')

common = static_library('evas_loader_common',
    generic_src,
    include_directories : config_dir,
    dependencies: [generic_deps, rt],
)

bin_ext=''
if sys_exe_extension == ''
  bin_ext='.'+sys_exe_extension
endif

foreach loader : generic_loaders
  generic_src = []
  generic_deps = []
  generic_support = []

  if get_option('evas-loaders-disabler').contains(loader) == false
    subdir(loader)

    tmp = executable('evas_image_loader.'+loader,
        generic_src,
        include_directories : config_dir + [include_directories('common')],
        link_with : common,
        dependencies: [eina, generic_deps],
        install_dir : join_paths(dir_lib, 'evas', 'utils'),
        install : true,
    )

    bin = join_paths(dir_lib, 'evas', 'utils', 'evas_image_loader.'+loader+bin_ext)
    evas_loader_map += [[bin, generic_support]]
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach
