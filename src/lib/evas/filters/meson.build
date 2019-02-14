subdir('lua')

evas_src += files([
  'evas_filter.c',
  'evas_filter_parser.c',
  'evas_filter_private.h',
  'evas_filter_utils.c',
])

luaold_interpreters = [
  ['lua',     ['>=5.1.0','<5.3.0']],
  ['lua51',   ['>=5.1.0','<5.2.0']],
  ['lua-5.1', ['>=5.1.0','<5.2.0']],
  ['lua5.1',  ['>=5.1.0','<5.2.0']],
  ['lua52',   ['>=5.2.0','<5.3.0']],
  ['lua-5.2', ['>=5.2.0','<5.3.0']],
  ['lua5.2',  ['>=5.2.0','<5.3.0']],
]

if get_option('lua-interpreter') == 'lua'
  config_h.set('ENABLE_LUA_OLD', '1')
  foreach l : luaold_interpreters
    lua = dependency(l[0], version: l[1], required:false)
    if lua.found() == true
      break
    endif
  endforeach
else
  lua = dependency(get_option('lua-interpreter'))
endif

if sys_osx == true and get_option('lua-interpreter') == 'luajit'
# luajit on macro is broken, this means we need to generate our own dependency with our arguments, a library later still needs to link to luajit for the pagesize argument thingy
  lua = declare_dependency(
    include_directories: include_directories(lua.get_pkgconfig_variable('includedir')),
    link_args: ['-L'+lua.get_pkgconfig_variable('libdir'), '-l'+lua.get_pkgconfig_variable('libname')]
  )
endif

evas_deps += lua
