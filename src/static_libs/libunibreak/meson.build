
libunibreak_src = [
  'unibreakbase.h',
  'unibreakdef.h',
  'linebreak.h',
  'linebreakdef.h',
  'wordbreakdef.h',
  'wordbreak.h',
  'unibreakbase.c',
  'unibreakdef.c',
  'linebreak.c',
  'linebreakdata.c',
  'linebreakdef.c',
  'wordbreak.c',
  'graphemebreak.c',
  'graphemebreak.h',
  'graphemebreakdef.h',
]

libunibreak_lib = static_library('libunibreak',
    libunibreak_src,
    include_directories : config_dir,
    install: false,
)

libunibreak = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: libunibreak_lib,
  dependencies: [eina],
)
