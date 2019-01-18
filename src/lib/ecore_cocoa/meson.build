ecore_cocoa_deps = [eina, ecore_input, ecore, emile, eet]
ecore_cocoa_pub_deps = [eina]
pub_eo_file_target = []

ecore_cocoa_header_src = [
  'Ecore_Cocoa.h'
]

ecore_cocoa_src = [
  'ecore_cocoa.m',
  'ecore_cocoa_keys.h',
  'ecore_cocoa_window.h',
  'ecore_cocoa_window.m',
  'ecore_cocoa_cnp.m',
  'ecore_cocoa_app.m',
  'ecore_cocoa_app.h',
  'ecore_cocoa_private.h'
]

evas_include_directories = [
  include_directories(join_paths('..', 'evas')),
  include_directories(join_paths('..', 'evas','canvas')),
  include_directories(join_paths('..', 'evas','common')),
  include_directories(join_paths('..', 'evas','common3d')),
  include_directories(join_paths('..', 'evas','common3d', 'save_load')),
  include_directories(join_paths('..', 'evas','include')),
  include_directories(join_paths('..', 'evas','filters'))
]

ecore_cocoa_lib = library('ecore_cocoa',
    ecore_cocoa_src, pub_eo_file_target,
    dependencies: ecore_cocoa_deps + ecore_cocoa_pub_deps,
    include_directories : config_dir + evas_include_directories,
    link_args : ['-lobjc', '-framework', 'CoreFoundation', '-framework', 'cocoa', '-Wl,-U,_evas_textblock_text_markup_to_utf8'],
    install: true,
    version : meson.project_version()
)

ecore_cocoa = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_cocoa_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_cocoa_pub_deps,
)

install_headers(ecore_cocoa_header_src,
  install_dir : dir_package_include,
)
