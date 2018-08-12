eo_function_overrides_src = [
  'function_overrides_inherit.c',
  'function_overrides_inherit.h',
  'function_overrides_inherit2.c',
  'function_overrides_inherit2.h',
  'function_overrides_inherit3.c',
  'function_overrides_inherit3.h',
  'function_overrides_main.c',
  'function_overrides_simple.c',
  'function_overrides_simple.h'
]

eo_function_overrides = executable('eo_function_overrides',
  eo_function_overrides_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-function-overrides', eo_interface,
  env : test_env
)
