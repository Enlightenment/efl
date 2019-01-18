ethumb_header_src = [
  'Ethumb.h'
]

ethumb_src = [
  'ethumb.c',
  'ethumb_private.h',
  'md5.h',
  'md5.c',
]

ethumb_deps = [eet, evas, ecore, ecore_evas, ecore_file, ecore_imf, edje, emile]
ethumb_pub_deps = [eina, eo, efl]

ethumb_lib = library('ethumb',
    ethumb_src,
    dependencies: ethumb_pub_deps + [m] + ethumb_deps,
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

ethumb = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ethumb_lib,
  dependencies: ethumb_pub_deps,
)

install_headers(ethumb_header_src,
  install_dir : dir_package_include,
)
