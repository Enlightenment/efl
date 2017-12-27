rg_etc_src = [
'etc2_encoder.c',
'rg_etc1.c',
'rg_etc1.h',
'rg_etc2.c',
]

rg_etc_lib = static_library('rg_etc', rg_etc_src,
  dependencies : eina,
  include_directories : config_dir,
)

rg_etc = declare_dependency(
  include_directories: include_directories('.'),
  link_with: rg_etc_lib
)
