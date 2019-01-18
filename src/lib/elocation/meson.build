elocation_deps = [eina, eo, efl, ecore, eldbus]
elocation_pub_deps = [ecore, eldbus]

elocation_src = files(['elocation.c'])

subdir('gen')

elocation_header_src = ['Elocation.h']


elocation_lib = library('elocation',
    elocation_src,
    dependencies: [m] + elocation_deps + elocation_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

elocation = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: elocation_lib,
  dependencies: elocation_pub_deps,
)

install_headers(elocation_header_src,
  install_dir : dir_package_include,
)
