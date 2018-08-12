#vg common is not really a static lib, is required by evas and requires evas ... so only collecting src here and compile them into evas ... ...

vg_common_src = files([
  'vg_common.c',
  'vg_common.h',
])

vg_common_inc_dir = include_directories('.')

vg_common = declare_dependency(
  include_directories: vg_common_inc_dir,
  sources: vg_common_src,
)
