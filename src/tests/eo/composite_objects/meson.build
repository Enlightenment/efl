eo_composite_objects_src = [
  'composite_objects_comp.c',
  'composite_objects_comp.h',
  'composite_objects_main.c',
  'composite_objects_simple.c',
  'composite_objects_simple.h'
]

eo_composite_objects = executable('eo_composite_objects',
  eo_composite_objects_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-composite-objects', eo_composite_objects,
  env : test_env
)

eo_composite_objects = executable('eo_composite_objects_dbg',
  eo_composite_objects_src,
  dependencies: [eo_dbg],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-composite-objects-dbg', eo_composite_objects,
  env : test_env
)
