eo_constructors_src = [
  'constructors_main.c',
  'constructors_mixin.c',
  'constructors_mixin.h',
  'constructors_simple.c',
  'constructors_simple.h',
  'constructors_simple2.c',
  'constructors_simple2.h',
  'constructors_simple3.c',
  'constructors_simple3.h',
  'constructors_simple4.c',
  'constructors_simple4.h',
  'constructors_simple5.c',
  'constructors_simple5.h',
  'constructors_simple6.c',
  'constructors_simple6.h',
  'constructors_simple7.c',
  'constructors_simple7.h'
]

eo_constructors = executable('eo_constructors',
  eo_constructors_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-constructors', eo_constructors,
  env : test_env
)

eo_constructors = executable('eo_constructors_dbg',
  eo_constructors_src,
  dependencies: [eo_dbg],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-constructors-dbg', eo_constructors,
  env : test_env
)
