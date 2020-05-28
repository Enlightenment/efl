eet_deps = [rg_etc]
eet_pub_deps = [eina, emile, efl]
eet_ext_deps = [crypto, jpeg, m, rg_etc_simple]

eet_header_src = [
  'Eet.h'
]

eet_src = files([
  'Eet_private.h',
  'eet_alloc.c',
  'eet_cipher.c',
  'eet_connection.c',
  'eet_data.c',
  'eet_dictionary.c',
  'eet_image.c',
  'eet_lib.c',
  'eet_node.c',
  'eet_utils.c',
])

eet_lib = library('eet',
    eet_src, pub_eo_file_target,
    c_args : package_c_args,
    dependencies: eet_deps + eet_pub_deps + eet_ext_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

eet = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: eet_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: eet_pub_deps,
)

install_headers(eet_header_src,
  install_dir : dir_package_include,
)
