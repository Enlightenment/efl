subdir('lua')

evas_src += files([
  'evas_filter.c',
  'evas_filter_parser.c',
  'evas_filter_private.h',
  'evas_filter_utils.c',
])

luajit = dependency('luajit')
if sys_osx == true
# luajit on macos is broken, this means we need to generate our own dependency with our arguments, a library later still needs to link to luajit for the pagesize argument thingy
  luajit = declare_dependency(
    include_directories: include_directories(luajit.get_pkgconfig_variable('includedir')),
    link_args: ['-L'+luajit.get_pkgconfig_variable('libdir'), '-l'+luajit.get_pkgconfig_variable('libname')]
  )
endif

evas_deps += luajit
