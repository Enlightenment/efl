ecore_input_deps = [ecore]
ecore_input_pub_deps = [eina, eo]

ecore_input_header_src = [
  'Ecore_Input.h'
]

ecore_input_src = [
  'ecore_input.c',
  'ecore_input_compose.c',
  'ecore_input_joystick.c',
  'ecore_input_compose.h',
  'ecore_input_private.h'
]

if get_option('eeze')
  ecore_input_deps += eeze
endif


ecore_input_lib = library('ecore_input',
    ecore_input_src, pub_eo_file_target,
    dependencies: ecore_input_pub_deps + ecore_input_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

ecore_input = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_input_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_input_pub_deps,
)

install_headers(ecore_input_header_src,
  install_dir : dir_package_include,
)
