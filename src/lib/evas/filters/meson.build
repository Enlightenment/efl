if get_option('lua-interpreter') != 'none'
  subdir('lua')

  evas_deps += lua
endif

evas_src += files([
  'evas_filter.c',
  'evas_filter_parser.c',
  'evas_filter_private.h',
  'evas_filter_utils.c',
])
