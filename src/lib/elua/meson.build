elua_deps = [eina, eo, efl, ecore, ecore_file, intl]
if sys_osx == true
# luajit on macro is broken, this means we need to generate our own dependency with our arguments, a library later still needs to link to luajit for the pagesize argument thingy
  luajit = dependency('luajit')
  lua = declare_dependency(
    include_directories: include_directories(luajit.get_pkgconfig_variable('includedir')),
    link_args: ['-L'+luajit.get_pkgconfig_variable('libdir'), '-l'+luajit.get_pkgconfig_variable('libname')]
  )
  elua_pub_deps = [lua]
else
  elua_pub_deps = [dependency('luajit')]
endif


elua_src = ['elua.c', 'io.c', 'cache.c']
elua_header_src = ['Elua.h']

elua_lib = library('elua',
    elua_src,
    dependencies: [m, dl] + elua_deps + elua_pub_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

elua = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: elua_lib,
  dependencies: elua_pub_deps,
)

install_headers(elua_header_src,
  install_dir : dir_package_include,
)
