engine_src = files([
  'evas_engine.c',
  'evas_engine.h',
  'evas_wl_main.c'
])

engine_deps = [ecore_wl2, dependency('wayland-egl')]

engine_include_dir = include_directories(join_paths('..','wayland_common'))