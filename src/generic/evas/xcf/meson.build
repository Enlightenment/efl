generic_src = files([
  'common.h',
  'main.c',
  'pixelfuncs.c'
])

if (not sys_windows)
  generic_deps = [eina, dependency('zlib')]
else
  generic_deps = [eina, zlib]
endif
generic_support = ['xcf.gz']
