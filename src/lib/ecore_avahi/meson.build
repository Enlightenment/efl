ecore_avahi_deps = []
ecore_avahi_pub_deps = [eina, ecore]

ecore_avahi_src = ['ecore_avahi.c']
ecore_avahi_header_src = ['Ecore_Avahi.h']

ecore_avahi_deps += dependency('avahi-client')
config_h.set('HAVE_AVAHI', '1')

ecore_avahi_lib = library('ecore_avahi',
    ecore_avahi_src,
    dependencies: [m] + ecore_avahi_deps + ecore_avahi_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    version : meson.project_version()
)

ecore_avahi = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_avahi_lib,
  dependencies: ecore_avahi_pub_deps,
)

install_headers(ecore_avahi_header_src,
  install_dir : dir_package_include,
)
