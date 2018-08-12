ecore_fb_deps = [ecore, ecore_input]
ecore_fb_pub_deps = [eina]

ecore_fb_header_src = [
  'Ecore_Fb.h'
]

ecore_fb_src = [
  'ecore_fb.c',
  'ecore_fb_li.c',
  'ecore_fb_ts.c',
  'ecore_fb_vt.c',
  'ecore_fb_keytable.h',
  'ecore_fb_private.h'
]

ecore_fb_lib = library('ecore_fb',
    ecore_fb_src, pub_eo_file_target,
    dependencies: ecore_fb_pub_deps + ecore_fb_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

ecore_fb = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_fb_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_fb_pub_deps,
)

install_headers(ecore_fb_header_src,
  install_dir : dir_package_include,
)
