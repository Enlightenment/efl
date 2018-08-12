lz4_src = [
'lz4.c',
'lz4hc.c'
]

lz4_lib = static_library('lz4', lz4_src,
  include_directories : config_dir,
)

lz4 = declare_dependency(
  include_directories: include_directories('.'),
  link_with: lz4_lib
)
