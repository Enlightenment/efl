efreet_header_src = [
  'Efreet.h',
  'efreet_base.h',
  'efreet_desktop.h',
  'efreet_icon.h',
  'efreet_ini.h',
  'efreet_menu.h',
  'efreet_utils.h',
  'efreet_uri.h',
  'Efreet_Mime.h',
  'Efreet_Trash.h'
]

efreet_src = [
  'efreet.c',
  'efreet_base.c',
  'efreet_icon.c',
  'efreet_xml.c',
  'efreet_ini.c',
  'efreet_desktop.c',
  'efreet_desktop_command.c',
  'efreet_menu.c',
  'efreet_utils.c',
  'efreet_uri.c',
  'efreet_cache.c',
  'efreet_private.h',
  'efreet_xml.h',
  'efreet_cache_private.h'
]

efreet_deps = [eet, ecore, ecore_con, ecore_file, eo, efl, eina, emile, ecore_ipc, buildsystem,intl]
efreet_pub_deps = [eina, eo, efl]

efreet_lib = library('efreet',
    efreet_src,
    dependencies: efreet_pub_deps + [m] + efreet_deps,
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : [package_c_args, '-DDATA_DIR="'+dir_data+'"'],
    version : meson.project_version()
)

efreet = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: efreet_lib,
  dependencies: efreet_pub_deps,
)

efreet_mime_lib = library('efreet_mime',
    'efreet_mime.c',
    dependencies: efreet_pub_deps + [m, efreet, efreet_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)


efreet_mime = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: efreet_mime_lib,
  dependencies: efreet_pub_deps
)

efreet_trash_lib = library('efreet_trash',
    'efreet_trash.c',
    dependencies: efreet_pub_deps + [efreet, efreet_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

efreet_trash = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: efreet_trash_lib,
  dependencies: efreet_pub_deps,
)

install_headers(efreet_header_src,
  install_dir : dir_package_include,
)

pkgconfig.generate(efreet_mime_lib,
        name : 'efreet-mime',
        subdirs : package_version_name,
        version : version_major + '.' + version_minor,
        libraries : [tmp_deps] + tmp_pub_deps,
)

pkgconfig.generate(efreet_trash_lib,
        name : 'efreet-trash',
        subdirs : package_version_name,
        version : version_major + '.' + version_minor,
        libraries : [tmp_deps] + tmp_pub_deps,
)
