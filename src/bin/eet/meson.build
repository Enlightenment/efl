_eet_bin = executable('eet',
  'eet_main.c',
  dependencies: [eet],
  install : true
)

install_data(['diffeet','vieet'],
  install_mode: 'rwxr-xr-x',
  install_dir : dir_bin
)

if meson.is_cross_build()
  eet_bin = find_program('eet', native : true)
else
  eet_bin = _eet_bin
endif
