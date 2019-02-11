eo_signals_src = [
  'signals_main.c',
  'signals_simple.c',
  'signals_simple.h'
]

eo_signals = executable('eo_signals',
  eo_signals_src,
  dependencies: [eo],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-signals', eo_signals,
  env : test_env
)

eo_signals = executable('eo_signals_dbg',
  eo_signals_src,
  dependencies: [eo_dbg],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eo-signals-dbg', eo_signals,
  env : test_env
)
