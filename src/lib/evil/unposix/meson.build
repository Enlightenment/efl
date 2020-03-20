project('evil_unposix', 'c')

if target_machine.system() == 'windows'
  subdir('sys')

  evil_unposix_src = ['fnmatch.h',
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

  evil_unposix_lib = library('evil_unposix', evil_unposix_src,
    include_directories: [include_directories('.')],
    dependencies: [],
    #install: true,
    #version: meson.project_version(),
  )

  evil_unposix = declare_dependency(
    include_directories: [include_directories('.')],
    dependencies: [],
    link_with: evil_unposix_lib,
  )

else
  evil_unposix = declare_dependency()

endif

automatic_pkgfile = false


