efl_deps = [m]
efl_pub_deps = [eo]
efl_ext_deps = []

efl_header_src = [
  'Efl.h',
  'Efl_MVVM_Common.h'
]

efl_src = []

subdir('interfaces')
package_header_subdirs += 'interfaces'

efl_lib = library('efl',
    efl_src, pub_eo_file_target,
    c_args : package_c_args,
    dependencies: [efl_deps, efl_pub_deps, efl_ext_deps],
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

package_eo_subdirs += ['interfaces']
