ecore_imf_deps = [ecore]
ecore_imf_pub_deps = [eina]

ecore_imf_header_src = [
   'Ecore_IMF.h'
]

ecore_imf_src = [
  'ecore_imf.c',
  'ecore_imf_context.c',
  'ecore_imf_module.c',
  'ecore_imf_private.h',
]


ecore_imf_lib = library('ecore_imf',
    ecore_imf_src, pub_eo_file_target,
    dependencies: [buildsystem, ecore_imf_deps, ecore_imf_pub_deps],
    include_directories : config_dir,
    c_args : package_c_args,
    install: true,
    version : meson.project_version()
)

ecore_imf = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_imf_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_imf_pub_deps,
)

install_headers(ecore_imf_header_src,
  install_dir : dir_package_include,
)
