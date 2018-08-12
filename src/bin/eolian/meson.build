eolian_gen_src = [
  'main.c',
  'main.h',
  'types.c',
  'types.h',
  'headers.c',
  'headers.h',
  'sources.c',
  'sources.h',
  'docs.c',
  'docs.h'
]

eolian_gen = executable('eolian_gen',
        eolian_gen_src,
        dependencies: eolian,
        install: true,
        c_args : package_c_args,
)
