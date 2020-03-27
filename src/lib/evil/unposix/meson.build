dir_package_include = '.'

if target_machine.system() == 'windows'
  subdir('sys')

  evil_unposix_src = []

  evil_unposix_header_src = [
    'fnmatch.h',
    'getopt.h',
    'libgen.h',
    'limits.h',
    'locale.h',
    'math.h',
    'pthread.h',
    'semaphore.h',
    'stdlib.h',
    'string.h',
    'strings.h',
    'unimplemented.h',
    'unistd.h',
  ]


  evil_unposix_lib = static_library('evil_unposix',
    evil_unposix_src,
    include_directories: [include_directories('.'), config_dir],
    dependencies : [],
    install: true,
    version: meson.project_version(),
  )

  evil_unposix = declare_dependency(
    include_directories: [include_directories('.')],
    dependencies: [],
    link_with: evil_unposix_lib,
  )

  install_headers(evil_unposix_header_src,
    install_dir : dir_package_include,
  )

else
  evil_unposix = declare_dependency()

endif

automatic_pkgfile = (sys_windows == false)
