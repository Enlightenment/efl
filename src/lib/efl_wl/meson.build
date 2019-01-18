efl_wl_deps = [ecore_wl2, ecore_input, ecore, ecore_evas, evas, emile, eo, efl, eina]
efl_wl_pub_deps = [evas, dependency('wayland-server', version : '>= 1.11.0'), dependency('xkbcommon', version : '>= 0.6.0')]

if (get_option('x11'))
  efl_wl_deps += [ecore_x]
  efl_wl_pub_deps += [dependency('xkbcommon-x11')]
endif

efl_wl_src = [
 'dmabuf.c',
 'efl_wl.c'
]
efl_wl_header_src = ['Efl_Wl.h']


efl_wl_lib = library('efl_wl',
    efl_wl_src,
    dependencies: [m, dl] + efl_wl_deps + efl_wl_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    version : meson.project_version()
)

efl_wl = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: efl_wl_lib,
  dependencies: efl_wl_pub_deps,
)

install_headers(efl_wl_header_src,
  install_dir : dir_package_include,
)
