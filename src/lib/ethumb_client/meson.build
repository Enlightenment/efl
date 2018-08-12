ethumb_client_header_src = [
  'Ethumb_Client.h'
]

ethumb_client_src = [
  'ethumb_client.c'
]

ethumb_client_deps = [
  eina,
  eldbus,
  ethumb,
  ecore
]

ethumb_client_lib = library('ethumb_client',
    ethumb_client_src,
    dependencies: [ethumb_client_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
)

ethumb_client = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ethumb_client_lib,
  sources : pub_eo_file_target,
  dependencies: [eina, eo, efl, ethumb],
)

install_headers(ethumb_client_header_src,
  install_dir : dir_package_include,
)
