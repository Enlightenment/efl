evas_include_directories += include_directories(join_paths('..', rel_evas_modules, 'filters'))

raw_evas_src = [
  'evas_engine_filter.h',
  'evas_filter_blend.c',
  'evas_filter_blur.c',
  'evas_filter_bump.c',
  'evas_filter_curve.c',
  'evas_filter_displace.c',
  'evas_filter_fill.c',
  'evas_filter_mask.c',
  'evas_filter_transform.c',
]

foreach file : raw_evas_src
  evas_src += files(join_paths('..', rel_evas_modules, 'filters', file))
endforeach
