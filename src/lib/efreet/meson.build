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

efreet_src = files([
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
  'efreet_cache_private.h',
  'efreet_mime.c',
  'efreet_trash.c'
])

efreet_deps = [eet, ecore, ecore_con, ecore_file, eo, efl, eina, emile, ecore_ipc, buildsystem]
efreet_pub_deps = [eina, eo, efl]
efreet_ext_deps = [buildsystem_simple, intl, m]

package_c_args += ['-DDATA_DIR="'+dir_data+'"']

efreet_lib = library('efreet',
    efreet_src,
    dependencies: efreet_pub_deps + efreet_ext_deps + efreet_deps,
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

efreet = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: efreet_lib,
  dependencies: efreet_pub_deps,
)

install_headers(efreet_header_src,
  install_dir : dir_package_include,
)


#Deprecated efreet libs
# everything and everyone should use efreet itself, efreet_mime and efreet_trash are contained in libefreet.so
# libefreet_mime.so and libefreet_trash.so will just have NEEDED on libefreet.so so the symbols get caught in like that

deprecated_efreet_mime_lib = library('efreet_mime',
    dependencies: [efreet],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    version : meson.project_version()
)

deprecated_efreet_trash_lib = library('efreet_trash',
    dependencies: efreet,
    include_directories : config_dir + [include_directories('.')],
    install: true,
    version : meson.project_version()
)
pkgconfig.generate(efreet_lib,
        name : 'efreet-mime',
        description : 'Deprecated, please just use efreet',
        subdirs : package_version_name,
        version : version_major + '.' + version_minor + '.' + version_micro,
        libraries : efreet_pub_deps,
)

pkgconfig.generate(efreet_lib,
        name : 'efreet-trash',
        description : 'Deprecated, please just use efreet',
        subdirs : package_version_name,
        version : version_major + '.' + version_minor + '.' + version_micro,
        libraries : efreet_pub_deps,
)
