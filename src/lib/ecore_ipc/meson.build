ecore_ipc_deps = [ecore, ecore_con]
ecore_ipc_pub_deps = [eina]

ecore_ipc_header_src = [
  'Ecore_Ipc.h'
]

ecore_ipc_src = [
  'ecore_ipc.c',
  'ecore_ipc_private.h',
]

ecore_ipc_lib = library('ecore_ipc',
    ecore_ipc_src, pub_eo_file_target,
    dependencies: ecore_ipc_deps + ecore_ipc_pub_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

ecore_ipc = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_ipc_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_ipc_pub_deps,
)

install_headers(ecore_ipc_header_src,
  install_dir : dir_package_include,
)
