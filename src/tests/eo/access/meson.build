eo_access_src = [
  'access_inherit.c',
  'access_inherit.h',
  'access_main.c',
  'access_simple.c',
  'access_simple.h',
  'access_simple_protected.h'
]

eo_access = executable('eo_access',
  eo_access_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-access', eo_access,
  is_parallel: true,
)
