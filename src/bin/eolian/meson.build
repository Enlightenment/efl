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

eolian_gen_bin = executable('eolian_gen',
        eolian_gen_src,
        dependencies: eolian,
        install: true,
        c_args : package_c_args,
)

eolian_gen_path = eolian_gen_bin.full_path()


if meson.is_cross_build()
  _eolian_gen_bin = find_program('eolian_gen', native : true)
  eolian_gen_path = _eolian_gen_bin.path()
else
  _eolian_gen_bin = eolian_gen_bin
  eolian_gen_path = _eolian_gen_bin.full_path()
endif

eolian_gen = [_eolian_gen_bin, '-S']
