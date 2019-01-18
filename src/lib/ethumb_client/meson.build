ethumb_client_header_src = [
  'Ethumb_Client.h'
]

ethumb_client_src = [
  'ethumb_client.c'
]

ethumb_client_deps = [
  eldbus,
  ecore
]

ethumb_client_pub_deps = [eina, eo, efl, ethumb]

ethumb_client_lib = library('ethumb_client',
    ethumb_client_src,
    dependencies: ethumb_client_pub_deps + ethumb_client_deps,
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

ethumb_client = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ethumb_client_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ethumb_client_pub_deps,
)

install_headers(ethumb_client_header_src,
  install_dir : dir_package_include,
)
