eolian_aux_pub_deps = [eolian]
eolian_aux_deps = []

eolian_aux_src = [
'eolian_aux.c'
]

eolian_aux_lib = library('eolian_aux', eolian_aux_src,
    include_directories: config_dir,
    dependencies: eolian,
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

eolian_aux = declare_dependency(
  include_directories: include_directories('.'),
  link_with : eolian_aux_lib,
  dependencies: eolian,
)

install_headers('Eolian_Aux.h',
   install_dir : dir_package_include
)
