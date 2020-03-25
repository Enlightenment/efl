evil_deps = []
evil_pub_deps = []
if target_machine.system() == 'windows'
  subdir('unposix')

  evil_src = [
    'evil_dlfcn.c',
    'evil_fcntl.c',
    'evil_langinfo.c',
    'evil_locale.c',
    'evil_main.c',
    'evil_mman.c',
    'evil_stdio.c',
    'evil_stdlib.c',
    'evil_string.c',
    'evil_time.c',
    'evil_unistd.c',
    'evil_util.c',
    'evil_private.h',
    'pcre/regex.h',
  ]

  psapi = cc.find_library('psapi')
  ole32 = cc.find_library('ole32')
  ws2_32 = cc.find_library('ws2_32')
  secur32 = cc.find_library('secur32')
  uuid = cc.find_library('uuid')

  evil_lib = library('evil', evil_src,
    dependencies : [psapi, ole32, ws2_32, secur32, uuid, regexp],
    include_directories : [config_dir],
    install: true,
    version: meson.project_version(),
  )

  evil = declare_dependency(
    include_directories: [include_directories('.')],
    dependencies : [psapi, ole32, ws2_32, secur32, uuid, regexp],
    link_with: evil_lib,
  )
else
  evil = declare_dependency()
endif

automatic_pkgfile = false
