if get_option('avahi')
  ecore_avahi_deps = []

  ecore_avahi_src = ['ecore_avahi.c']
  ecore_avahi_header_src = ['Ecore_Avahi.h']

  ecore_avahi_deps += dependency('avahi-client')
  config_h.set('HAVE_AVAHI', '1')

  ecore_avahi_lib = library('ecore-avahi',
      ecore_avahi_src,
      dependencies: [ecore, m, ecore_avahi_deps],
      include_directories : config_dir + [include_directories(join_paths('..','..'))],
      install: true,
  )

  ecore_avahi = declare_dependency(
    include_directories: [include_directories('.')],
    link_with: ecore_avahi_lib,
    dependencies: [eina, ecore],
  )

  install_headers(ecore_avahi_header_src,
    install_dir : dir_package_include,
  )
else

endif 