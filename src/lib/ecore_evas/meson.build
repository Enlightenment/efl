ecore_evas_deps = [eina, ecore_input_evas, ecore_input, ecore, eet, evas, efl, eo, eina, emile]
pub_eo_file_target = []

ecore_evas_header_src = [
  'Ecore_Evas_Types.h',
  'Ecore_Evas.h',

]

ecore_evas_src = [
  'ecore_evas.c',
  'ecore_evas_buffer.c',
  'ecore_evas_buffer.h',
  'ecore_evas_deprecated.c',
  'ecore_evas_drm.h',
  'ecore_evas_ews.c',
  'ecore_evas_module.c',
  'ecore_evas_private.h',
  'ecore_evas_extn.h',
  'ecore_evas_extn.c',
  'ecore_evas_wayland.h',
  'ecore_evas_cocoa.h',
  'ecore_evas_win32.h',
  'ecore_evas_x11.h',
  'ecore_evas_util.c'
]


ecore_evas_lib = library('ecore_evas',
    ecore_evas_src, pub_eo_file_target,
    dependencies: [ecore_evas_deps, buildsystem],
    include_directories : config_dir,
    install: true,
)

ecore_evas = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_evas_lib,
  sources : pub_eo_file_target,
  dependencies: [evas],
)

install_headers(ecore_evas_header_src,
  install_dir : dir_package_include,
)