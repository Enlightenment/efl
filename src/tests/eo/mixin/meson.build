eo_mixin_src = [
  'mixin_inherit.c',
  'mixin_inherit.h',
  'mixin_main.c',
  'mixin_mixin.c',
  'mixin_mixin.h',
  'mixin_mixin2.c',
  'mixin_mixin2.h',
  'mixin_mixin3.c',
  'mixin_mixin3.h',
  'mixin_mixin4.c',
  'mixin_mixin4.h',
  'mixin_simple.c',
  'mixin_simple.h'
]

eo_mixin = executable('eo_mixin',
  eo_mixin_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-mixins', eo_mixin,
  env : test_env
)
eo_mixin = executable('eo_mixin_dbg',
  eo_mixin_src,
  dependencies: [eo_dbg],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-mixins-dbg', eo_mixin,
  env : test_env
)
