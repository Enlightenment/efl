
libunibreak_src = [
  'unibreakbase.h',
  'unibreakdef.h',
  'linebreak.h',
  'linebreakdef.h',
  'wordbreakdef.h',
  'wordbreak.h',
  'wordbreakdata.c',
  'unibreakbase.c',
  'unibreakdef.c',
  'linebreak.c',
  'linebreakdata.c',
  'linebreakdef.c',
  'wordbreak.c'
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
