eolian_aux_test_src = [
'eolian_aux.c',
'eolian_aux_suite.c',
'eolian_aux_suite.h'
]

eolian_aux_suite = executable('eolian_aux_suite',
  eolian_aux_test_src,
  dependencies: [eolian, eolian_aux, check],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eolian_aux', eolian_aux_suite,
  timeout: 30,
  env : test_env
)
