#vg common is not really a static lib, is required by evas and requires evas ... so only collecting src here and compile them into evas ... ...

vg_common_src = files([
  'vg_common_svg.c',
  'vg_common.h',
])

json = dependency('rlottie', required: get_option('evas-loaders-disabler').contains('json') == false)

if (json.found())
  config_h.set('BUILD_VG_LOADER_JSON', '1')
  vg_common_src += files('vg_common_json.c')
endif

vg_common_inc_dir = include_directories('.')

vg_common = declare_dependency(
  include_directories: vg_common_inc_dir,
  sources: vg_common_src,
)
