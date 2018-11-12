
draw_src = [
  'draw_main.c',
  'draw_main_neon.c',
  'draw_main.c',
  'draw_convert.c',
  'draw_alpha_main.c'
]

draw_opt_lib = [ ]

if cpu_sse3 == true
  draw_opt = static_library('draw_opt',
    sources: [ 'draw_main_sse2.c' ],
    include_directories: config_dir + [include_directories(join_paths('..', '..', 'lib'))],
    c_args: draw_opt_c_args,
    dependencies : [eina, efl]
  )
  draw_opt_lib += [ draw_opt ]
else
  draw_src += [ 'draw_main_sse2.c' ]
endif

draw_lib = static_library('draw',
    draw_src,
    dependencies : [eina, efl],
    include_directories : config_dir + [include_directories(join_paths('..', '..', 'lib'))],
    install: false,
    link_with: draw_opt_lib,
)

draw = declare_dependency(
  include_directories: [include_directories('.'), include_directories(join_paths('..', '..', 'lib'))],
  link_with: draw_lib,
  dependencies: [eina],
)
