
draw_src = [
  'draw_main_sse2.c',
  'draw_main.c',
  'draw_main_neon.c',
  'draw_main.c',
  'draw_convert.c',
  'draw_alpha_main.c'
]


draw_lib = static_library('draw',
    draw_src,
    dependencies : [eina, efl],
    include_directories : config_dir + [include_directories(join_paths('..', '..', 'lib'))],
    install: false,
)

draw = declare_dependency(
  include_directories: [include_directories('.'), include_directories(join_paths('..', '..', 'lib'))],
  link_with: draw_lib,
  dependencies: [eina],
)
