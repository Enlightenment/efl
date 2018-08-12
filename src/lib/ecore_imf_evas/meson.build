ecore_imf_evas_header_src = [
  'Ecore_IMF_Evas.h'
]

ecore_imf_evas_src = [
  'ecore_imf_evas.c'
]

ecore_imf_evas_deps = [ecore_imf, evas]

ecore_imf_evas_lib = library('ecore_imf_evas',
    ecore_imf_evas_src,
    dependencies: [eina, eo, efl, m, ecore_imf_evas_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
)

ecore_imf_evas = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_imf_evas_lib,
  sources : pub_eo_file_target,
  dependencies: [eina, eo, efl],
)

install_headers(ecore_imf_evas_header_src,
  install_dir : dir_package_include,
)
