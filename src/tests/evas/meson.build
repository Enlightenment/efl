evas_suite_src = [
  'evas_suite.c',
  'evas_test_new.c',
  'evas_test_init.c',
  'evas_test_object.c',
  'evas_test_object_smart.c',
  'evas_test_textblock.c',
  'evas_buffer_helper.c',
  'evas_test_text.c',
  'evas_test_callbacks.c',
  'evas_test_render_engines.c',
  'evas_test_filters.c',
  'evas_test_image.c',
  'evas_test_mask.c',
  'evas_test_evasgl.c',
  'evas_test_focus.c',
  'evas_test_events.c',
  'evas_tests_helpers.h',
  'evas_suite.h',
  'efl_test_canvas.c',
  'efl_test_canvas2.c',
  'efl_test_canvas3.c',
  'efl_canvas_animation.c',
  'evas_test_map.c',
  'evas_test_premul.c',
]

evas_suite = executable('evas_suite',
  evas_suite_src,
  dependencies: [evas_bin, evas, ecore_evas, dl, check, evas_ext_none_static_deps, eet], #external deps needed here since tests do include internal headers
  include_directories: include_directories(join_paths('..', '..', 'modules', 'evas', 'engines', 'buffer')),
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('evas-suite', evas_suite,
  env : test_env,
  timeout : master_timeout
)

# Compares every C/NEON pair in the evas span and point op tables. It includes
# the op translation units directly rather than linking them, because the copy
# tables are static; that keeps the library itself untouched. Skips (exit 77)
# on builds without BUILD_NEON.
if cpu_neon
  evas_neon_ops = executable('evas_neon_ops',
    ['evas_test_simd_ops.c'],
    dependencies: [evas_bin, evas, evas_ext_none_static_deps, eet], #internal headers, as above
    c_args : ['-DEVAS_BUILD', '-DSIMD_TIER=CPU_NEON', '-DSIMD_NAME="neon"']
  )

  test('evas-neon-ops', evas_neon_ops,
    env : test_env,
    timeout : master_timeout
  )

  # Same idea for the gaussian blur steps. They are static inline templates
  # rather than table entries, so this instantiates each one twice - from the
  # C file and from the NEON one - and diffs the pair.
  evas_neon_blur = executable('evas_neon_blur',
    ['evas_test_neon_blur.c'],
    dependencies: [evas_bin, evas, evas_ext_none_static_deps, eet, m],
    include_directories: include_directories(
      join_paths('..', '..', 'modules', 'evas', 'engines', 'software_generic', 'filters')),
    c_args : ['-DEVAS_BUILD']
  )

  test('evas-neon-blur', evas_neon_blur,
    env : test_env,
    timeout : master_timeout
  )
endif

# Compares every C/AVX2 pair in the evas blend op tables. Same harness as the
# NEON build, with the tier taken as a build macro. Needs -mavx2 for the AVX2
# kernels it pulls in through the op translation unit. op_blend_master_avx2.c
# is added as an extra source (not #included by evas_test_simd_ops.c) so its
# evas_common_op_blend_init_avx2() and the op_blend_span_funcs table it writes
# to resolve within this executable rather than against libevas's copy.
if cpu_avx2
  evas_avx2_ops = executable('evas_avx2_ops',
    ['evas_test_simd_ops.c',
     '../../lib/evas/common/evas_op_blend/op_blend_master_avx2.c',
     '../../lib/evas/common/evas_op_blend/op_blend_master_sse3.c',
     '../../lib/evas/common/evas_op_copy/op_copy_master_avx2.c',
     'evas_test_simd_avx2_alpha_stub.c'],
    dependencies: [evas_bin, evas, evas_ext_none_static_deps, eet],
    c_args : ['-DEVAS_BUILD', '-DSIMD_TIER=CPU_AVX2', '-DSIMD_NAME="avx2"'] + avx2_c_args
  )

  test('evas-avx2-ops', evas_avx2_ops,
    env : test_env,
    timeout : master_timeout
  )

  # Same harness, same executable sources, but compared against CPU_SSE3
  # instead of CPU_C. Some AVX2 kernels are ported operation-for-operation
  # from an SSE3 original that itself diverges from the plain-C reference
  # macros (a pre-existing property of the shipped SSE3 code, not something
  # introduced by the AVX2 port); for those kernels "bit-exact against
  # SSE3" is the correct acceptance bar, and this target answers that
  # question directly rather than folding it into the AVX2-vs-C result.
  #
  # -DSIMD_REQUIRE_EXACT makes this target's gate reject ANY in-span
  # difference, of any magnitude - this is the build meant to enforce
  # bit-exactness, so it must not tolerate the "max 1 LSB is fine" leniency
  # the vs-C build needs for the known SSE3-inherited divergence. See the
  # SIMD_REQUIRE_EXACT comment in evas_test_simd_ops.c.
  evas_avx2_vs_sse3_ops = executable('evas_avx2_vs_sse3_ops',
    ['evas_test_simd_ops.c',
     '../../lib/evas/common/evas_op_blend/op_blend_master_avx2.c',
     '../../lib/evas/common/evas_op_blend/op_blend_master_sse3.c',
     '../../lib/evas/common/evas_op_copy/op_copy_master_avx2.c',
     'evas_test_simd_avx2_alpha_stub.c'],
    dependencies: [evas_bin, evas, evas_ext_none_static_deps, eet],
    c_args : ['-DEVAS_BUILD', '-DSIMD_TIER=CPU_AVX2', '-DSIMD_NAME="avx2"',
              '-DSIMD_REF_TIER=CPU_SSE3', '-DSIMD_REF_NAME="sse3"',
              '-DSIMD_REQUIRE_EXACT=1'] + avx2_c_args
  )

  test('evas-avx2-vs-sse3-ops', evas_avx2_vs_sse3_ops,
    env : test_env,
    timeout : master_timeout
  )
endif
