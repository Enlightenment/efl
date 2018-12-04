
draw_src = [
  'draw_main.c',
  'draw_main_neon.c',
  'draw_main.c',
  'draw_convert.c',
  'draw_alpha_main.c'
]

draw_opt_lib = [ ]

if cpu_sse3 == true and sys_windows == false
  draw_opt = static_library('draw_opt',
    sources: [ 'draw_main_sse2.c' ],
    include_directories: config_dir + [include_directories(join_paths('..', '..', 'lib'))],
    c_args: native_arch_opt_c_args,
    dependencies : [eina, efl]
  )
  draw_opt_lib += [ draw_opt ]
else
  draw_src += [ 'draw_main_sse2.c' ]
endif

draw = declare_dependency(
  include_directories: [include_directories('.'), include_directories(join_paths('..', '..', 'lib'))],
  dependencies: [eina, efl],
  sources : draw_src,
  link_with : draw_opt_lib
)
