ethumb_header_src = [
  'Ethumb.h',
  'Ethumb_Plugin.h'
]

ethumb_src = [
  'ethumb.c',
  'ethumb_private.h',
  'md5.h',
  'md5.c',
]

ethumb_deps = [eet, evas, ecore, ecore_evas, ecore_file, ecore_imf, edje, emile]

ethumb_lib = library('ethumb',
    ethumb_src,
    dependencies: [eina, eo, efl, m, ethumb_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
)

ethumb = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ethumb_lib,
  dependencies: [eina, eo, efl],
)

install_headers(ethumb_header_src,
  install_dir : dir_package_include,
)
