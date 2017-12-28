
freetype_src = [
  'sw_ft_math.c',
  'sw_ft_math.h',
  'sw_ft_raster.c',
  'sw_ft_raster.h',
  'sw_ft_stroker.c',
  'sw_ft_stroker.h',
  'sw_ft_types.h',
]


freetype_lib = static_library('freetype',
    freetype_src,
    include_directories : config_dir,
    install: false,
)

freetype = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: freetype_lib,
  dependencies: [eina],
)
