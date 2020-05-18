embryo_deps = [ecore]
embryo_pub_deps = [eina, eo, efl]


embryo_header_src = [
  'Embryo.h'
]

embryo_src = [
  'embryo_amx.c',
  'embryo_args.c',
  'embryo_float.c',
  'embryo_main.c',
  'embryo_rand.c',
  'embryo_str.c',
  'embryo_time.c',
  'embryo_private.h'
]

embryo_deps = [eina, m]

embryo_lib = library('embryo',
    embryo_src,
    dependencies: [m, buildsystem] + embryo_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

embryo = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: embryo_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: embryo_pub_deps,
)

install_headers(embryo_header_src,
  install_dir : dir_package_include,
)
