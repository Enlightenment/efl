generic_loaders = [ 'pdf', 'ps', 'raw', 'rsvg', 'xcf' ]

if get_option('gstreamer') == true
  generic_loaders += [ 'gst' ]
endif

generic_src = []
generic_deps = []

subdir('common')

inc_dir = config_dir
common_deps = generic_deps
if (sys_windows)
  inc_dir = [inc_dir, zlib_include_dir]
  assert(inc_dir[0] == config_dir)
  assert(inc_dir[1] == zlib_include_dir)
else
  common_deps = [generic_deps, rt]
  assert(common_deps[0] == generic_deps)
  assert(common_deps[1] == rt)
endif

common = static_library('evas_loader_common',
    generic_src,
    include_directories : inc_dir,
    dependencies: common_deps,
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
        dependencies: [eina, generic_deps, evil_unposix],
        install_dir : join_paths(dir_lib, 'evas', 'utils'),
        install : true,
    )

    bin = join_paths(dir_lib, 'evas', 'utils', 'evas_image_loader.'+loader+bin_ext)
    evas_loader_map += [[bin, generic_support]]
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach
