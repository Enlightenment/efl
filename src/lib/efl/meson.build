efl_deps = []
efl_pub_deps = [eo]

efl_header_src = [
  'Efl.h',
  'Efl_Model_Common.h'
]

efl_src = []

subdir('interfaces')

efl_lib = library('efl',
    efl_src, pub_eo_file_target,
    dependencies: [eina, eo, m],
    install: true,
    version : meson.project_version()
)

efl = declare_dependency(
  include_directories: [include_directories('.'), config_dir],
  link_with: efl_lib,
  dependencies: efl_deps + efl_pub_deps,
  sources : pub_eo_file_target
)

install_headers(efl_header_src,
  install_dir : dir_package_include,
)

efl_eo_subdirs = ['interfaces']
