if target_machine.system() == 'windows'

  evil_unposix_sys_src = [
    'time.c',
  ]

  evil_unposix_sys_header_src = [
    'time.h', 
    'stat.h', 
    'types.h'
  ]

  evil_unposix_sys_lib = library('evil_unposix_sys',
    evil_unposix_sys_src,
    #version: 'meson.project_version(),
    install: true,
  )
  
  evil_unposix_sys = declare_dependency(
    include_directories: [include_directories('.')],
    dependencies: [],
    link_with: evil_unposix_sys_lib,
  )

  install_headers(evil_unposix_sys_header_src,
    install_dir : dir_package_include,
  ) 

endif

automatic_pkgfile = false
