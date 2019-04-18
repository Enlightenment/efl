evil_deps = []
evil_pub_deps = []
if target_machine.system() == 'windows'
  evil_header_src = [
  'Evil.h',
  'evil_dlfcn.h',
  'evil_fcntl.h',
  'evil_langinfo.h',
  'evil_locale.h',
  'evil_macro.h',
  'evil_macro_pop.h',
  'evil_macro_wrapper.h',
  'evil_main.h',
  'evil_stdio.h',
  'evil_stdlib.h',
  'evil_string.h',
  'evil_time.h',
  'evil_unistd.h',
  'evil_util.h',
  'dirent.h',
  'fnmatch.h',
  'pwd.h',
  'regex/regex.h'
  ]
  evil_header_sys_src = [join_paths('sys','mman.h')]

  evil_src = [
  'evil_dlfcn.c',
  'evil_fcntl.c',
  'evil_fnmatch.c',
  'evil_fnmatch_list_of_states.c',
  'evil_langinfo.c',
  'evil_locale.c',
  'evil_main.c',
  'evil_mman.c',
  'evil_pwd.c',
  'evil_stdio.c',
  'evil_stdlib.c',
  'evil_string.c',
  'evil_time.c',
  'evil_unistd.c',
  'evil_util.c',
  'evil_private.h',
  'evil_fnmatch_private.h',
  ]

  subdir('regex')

  psapi = cc.find_library('psapi')
  ole32 = cc.find_library('ole32')
  ws2_32 = cc.find_library('ws2_32')
  secur32 = cc.find_library('secur32')
  uuid = cc.find_library('uuid')

  evil_lib = library('evil', evil_src,
    dependencies : [psapi, ole32, ws2_32, secur32, uuid],
    include_directories : [config_dir, include_directories('regex')],
  )

  evil = declare_dependency(
    include_directories: [config_dir, include_directories('regex'), include_directories('.')],
    dependencies : [psapi, ole32, ws2_32, secur32, uuid],
    link_with: evil_lib,
  )
else
  evil = declare_dependency()
  automatic_pkgfile = false
endif
