ecore_sdl_deps = [eina, ecore, ecore_input]
ecore_sdl_pub_deps = []

ecore_sdl_src = [
  'ecore_sdl.c'
]

ecore_sdl_header_src = [
  'Ecore_Sdl.h',
  'Ecore_Sdl_Keys.h'
]

ecore_sdl_deps += dependency('sdl2')

ecore_sdl_lib = library('ecore_sdl',
    ecore_sdl_src,
    dependencies: [m] + ecore_sdl_deps + ecore_sdl_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
)

ecore_sdl = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_sdl_lib,
  dependencies: ecore_sdl_pub_deps,
)

install_headers(ecore_sdl_header_src,
  install_dir : dir_package_include,
)
