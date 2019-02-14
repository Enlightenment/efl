elua_deps = [eina, eo, efl, ecore, ecore_file, intl]
elua_pub_deps = [dependency('luajit')]

elua_src = ['elua.c', 'io.c', 'cache.c']
elua_header_src = ['Elua.h']

elua_lib = library('elua',
    elua_src,
    dependencies: [m, dl] + elua_deps + elua_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

elua = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: elua_lib,
  dependencies: elua_pub_deps,
)

install_headers(elua_header_src,
  install_dir : dir_package_include,
)
