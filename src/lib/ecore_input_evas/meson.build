ecore_input_evas_deps = [ecore, ecore_input, evas]
pub_eo_file_target = []

ecore_input_evas_header_src = [
   'Ecore_Input_Evas.h'
]

ecore_input_evas_src = [
  'ecore_input_evas.c',
  'ecore_input_evas_private.h',
]


ecore_input_evas_lib = library('ecore_input_evas',
    ecore_input_evas_src, pub_eo_file_target,
    dependencies: [ ecore_input_evas_deps],
    include_directories : config_dir,
    install: true,
)

ecore_input_evas = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_input_evas_lib,
  sources : pub_eo_file_target,
  dependencies: [eina, evas],
)

install_headers(ecore_input_evas_header_src,
  install_dir : dir_package_include,
)