dir_package_include = '.'

if sys_windows
  evil_unposix_src = []

  evil_unposix_header_src = [
    'libgen.h',
    'limits.h',
    'locale.h',
    'math.h',
    'pthread.h',
    'stdlib.h',
    'string.h',
    'strings.h',
    'unimplemented.h',
    'unistd.h',
  ]

  subdir('sys')

  evil_unposix_lib = static_library('evil_unposix',
    evil_unposix_src,
    include_directories: [include_directories('.'), config_dir],
    dependencies : [getopt_dep],
    install: true,
    version: meson.project_version(),
  )

  evil_unposix = declare_dependency(
    include_directories: [include_directories('.')],
    dependencies: [getopt_dep],
    link_with: evil_unposix_lib,
  )

  install_headers(evil_unposix_header_src,
    install_dir : dir_package_include,
  )
else
  evil_unposix = declare_dependency()
endif

automatic_pkgfile = (sys_windows == false)
