generic_loaders = [ 'pdf', 'ps', 'raw', 'rsvg', 'xcf' ]

if get_option('gstreamer') == true
  generic_loaders += [ 'gst' ]
endif

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

    # And the same links in the build tree, so that a video or a document
    # has a picture when EFL is run uninstalled. Made now, pointing at a
    # name that is built in a moment; a link is followed when it is used,
    # not when it is made.
    #
    # Not bin_ext: that is '.' on anything without an executable suffix, and
    # the install script strips the dot back off again. The name here has to
    # be the one the binary actually has.
    if generic_support.length() > 0
      intree_name = 'evas_image_loader.'+loader
      if sys_exe_extension != ''
        intree_name = intree_name + '.' + sys_exe_extension
      endif
      run_command(join_paths(meson.project_source_root(), 'meson', 'evas_loader_intree.sh'),
                  meson.current_build_dir(),
                  intree_name,
                  generic_support,
                  check: true)
    endif
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach
