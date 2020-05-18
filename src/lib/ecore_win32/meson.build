if sys_windows == true
  ecore_win32_deps = [evil, eina, ecore, ecore_input]
  ecore_win32_pub_deps = []

  ecore_win32_src = [
    'ecore_win32.c',
    'ecore_win32_clipboard.c',
    'ecore_win32_cursor.c',
    'ecore_win32_dnd.c',
    'ecore_win32_dnd_data_object.cpp',
    'ecore_win32_dnd_drop_source.cpp',
    'ecore_win32_dnd_drop_target.cpp',
    'ecore_win32_dnd_enumformatetc.cpp',
    'ecore_win32_event.c',
    'ecore_win32_monitor.c',
    'ecore_win32_window.c',
  ]

  ecore_win32_header_src = [
    'Ecore_Win32.h'
  ]

  ecore_win32_lib = library('ecore_win32',
      ecore_win32_src,
      dependencies: ecore_win32_deps + ecore_win32_pub_deps,
      include_directories : config_dir,
      install: true,
      c_args : package_c_args,
  )

  ecore_win32 = declare_dependency(
    include_directories: [include_directories('.')],
    link_with: ecore_win32_lib,
    dependencies: ecore_win32_pub_deps,
  )

  install_headers(ecore_win32_header_src,
    install_dir : dir_package_include,
  )
else
  automatic_pkgfile = false
endif
