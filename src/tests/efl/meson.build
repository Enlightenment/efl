efl_suite_src = [
  'efl_suite.c',
  'efl_suite.h',
  'efl_test_model_composite.c',
  'efl_test_model_container.c'
]

efl_suite_bin = executable('efl_suite',
  efl_suite_src,
  dependencies: [efl, ecore],
)

test('efl-suite', efl_suite_bin)
