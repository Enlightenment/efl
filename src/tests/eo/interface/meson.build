eo_interface_src = [
  'interface_interface.c',
  'interface_interface.h',
  'interface_interface2.c',
  'interface_interface2.h',
  'interface_main.c',
  'interface_simple.c',
  'interface_simple.h'
]

eo_interface = executable('eo_interface',
  eo_interface_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-interface', eo_interface,
  env : test_env
)
