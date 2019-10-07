#fixed dependencies by efl
png = dependency('libpng')
tiff = dependency('libtiff-4', required: get_option('evas-loaders-disabler').contains('tiff') == false)
giflib = cc.find_library('gif')
webp = dependency('libwebp', required: get_option('evas-loaders-disabler').contains('webp') == false)

evas_loader_saver_mod_bmp     = ['bmp',     'shared', []]
evas_loader_saver_mod_eet     = ['eet',     'static', [eet]]
evas_loader_saver_mod_generic = ['generic', 'shared', [rt]]
evas_loader_saver_mod_gif     = ['gif',     'shared', [giflib]]
evas_loader_saver_mod_ico     = ['ico',     'shared', []]
evas_loader_saver_mod_jpeg    = ['jpeg',    'static', [jpeg]]
evas_loader_saver_mod_pmaps   = ['pmaps',   'shared', []]
evas_loader_saver_mod_png     = ['png',     'static', [png]]
evas_loader_saver_mod_psd     = ['psd',     'shared', []]
evas_loader_saver_mod_tga     = ['tga',     'shared', []]
evas_loader_saver_mod_tgv     = ['tgv',     'shared', [rg_etc, lz4]]
evas_loader_saver_mod_tiff    = ['tiff',    'shared', [tiff]]
evas_loader_saver_mod_wbmp    = ['wbmp',    'shared', []]
evas_loader_saver_mod_webp    = ['webp',    'shared', [webp]]
evas_loader_saver_mod_xpm     = ['xpm',     'shared', []]

evas_image_loaders_file = [
  evas_loader_saver_mod_bmp,
  evas_loader_saver_mod_eet,
  evas_loader_saver_mod_generic,
  evas_loader_saver_mod_gif,
  evas_loader_saver_mod_ico,
  evas_loader_saver_mod_jpeg,
  evas_loader_saver_mod_pmaps,
  evas_loader_saver_mod_png,
  evas_loader_saver_mod_psd,
  evas_loader_saver_mod_tga,
  evas_loader_saver_mod_tgv,
  evas_loader_saver_mod_tiff,
  evas_loader_saver_mod_wbmp,
  evas_loader_saver_mod_webp,
  evas_loader_saver_mod_xpm
]

evas_image_savers_file = [
  evas_loader_saver_mod_eet,
  evas_loader_saver_mod_jpeg,
  evas_loader_saver_mod_png,
  evas_loader_saver_mod_tgv,
  evas_loader_saver_mod_tiff,
  evas_loader_saver_mod_webp
]

foreach loader_inst : evas_image_loaders_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    if loader_type == 'static'
      config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
      file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'image_loaders', loader, 'evas_image_load_'+loader+'.c')
      config_h.set('EVAS_STATIC_BUILD_'+loader.to_upper(), '1')
      tmp = static_library('image_loader_'+loader, file,
        include_directories : config_dir,
        dependencies : [evas_pre] + loader_deps
      )
      evas_static_list += [declare_dependency(
        sources: file,
        dependencies: loader_deps,
      )]
    endif
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach

foreach loader_inst : evas_image_savers_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    if loader_type == 'static'
      config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
      file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'image_savers', loader, 'evas_image_save_'+loader+'.c')
      config_h.set('EVAS_STATIC_BUILD_'+loader.to_upper(), '1')
      tmp = static_library('image_saver_'+loader, file,
        include_directories : config_dir,
        dependencies : [evas_pre] + loader_deps
      )
      evas_static_list += [declare_dependency(
        sources: file,
        dependencies: loader_deps,
      )]
    endif
  else
    message('Image saver '+loader+' disabled')
  endif
endforeach

evas_lib = library('evas',
    'empty.c',
    #all the source code is coming from evas_pre_lib_dep
    dependencies: [evas_pre_lib_dep, evas_static_list],
    install: true,
    c_args : '-DPACKAGE_DATA_DIR="'+join_paths(dir_data, 'evas')+'"',
    version : meson.project_version()
)

evas_pub_deps = [eina, ecore, ector, emile, evas_pre]

evas = declare_dependency(
   link_with : [evas_lib],
   dependencies : evas_pub_deps,
   include_directories : evas_include_directories
)

evas_bin = declare_dependency(
   link_with : [evas_lib],
   dependencies : [eina, ecore, ector, emile, lua],
   include_directories : evas_include_directories
)

tmp_package_subdirs = []
automatic_pkgfile = false

foreach subdir : evas_header_subdirs
  tmp_package_subdirs += join_paths('evas-'+version_major, subdir)
endforeach

pkgconfig.generate(evas_lib,
  name : 'evas',
  subdirs : ['evas-'+version_major] + tmp_package_subdirs,
  version : version_major + '.' + version_minor + '.' + version_micro,
  libraries : [eina, ecore, ector, emile, lua],
)

foreach loader_inst : evas_image_loaders_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    if loader_type == 'shared'
      config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
      file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'image_loaders', loader, 'evas_image_load_'+loader+'.c')
      evas_package_modules = join_paths(dir_lib, 'evas', 'modules')
      mod_install_dir = join_paths(evas_package_modules, 'image_loaders', loader, version_name)
      shared_module('shared_loader_'+loader, file,
        include_directories : config_dir,
        dependencies : [eina, evas] + loader_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'libshared_loader_'+loader + '.' + sys_mod_extension)
    endif
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach

foreach loader_inst : evas_image_savers_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    if loader_type == 'shared'
      config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
      file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'image_savers', loader, 'evas_image_save_'+loader+'.c')
      evas_package_modules = join_paths(dir_lib, 'evas', 'modules')
      mod_install_dir = join_paths(evas_package_modules, 'image_savers', loader, version_name)
      shared_module('shared_saver_'+loader, file,
        include_directories : config_dir,
        dependencies : [eina, evas] + loader_deps,
        install : true,
        install_dir : mod_install_dir,
        name_suffix : sys_mod_extension
      )
      module_files += join_paths(mod_install_dir, 'libshared_saver_'+loader + '.' + sys_mod_extension)
    endif
  else
    message('Image saver '+loader+' disabled')
  endif
endforeach
