eo_children_src = [
  'children_main.c',
  'children_simple.c',
  'children_simple.h'
]

eo_children = executable('eo_children',
  eo_children_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-children', eo_children,
  env : test_env
)
