/* AVX2 copy kernels.
 *
 * This translation unit is compiled with -mavx2, exactly like
 * op_blend_master_avx2.c next door in evas_op_blend/ - see that file's
 * header for why AVX2 code must live in its own TU and be gated at runtime
 * by CPU_FEATURE_AVX2.
 *
 * Unlike blend, the copy op table has NO SSE3 kernels at all (see the
 * per-file header comments in op_copy_pixel_avx2.c / op_copy_color_avx2.c):
 * there is nothing to match bit-for-bit except the plain-C reference, which
 * is also the correctness bar copy already has for every other tier (copy
 * is exact, not a rounding op). So this master file does not need the
 * SSE3 statics or NEED_SSE3 dance that op_blend_master_avx2.c carries for
 * its 4-wide SSE3-body stage - the kernels in this group are written
 * directly against AVX2 intrinsics with no SSE3 fallback tier.
 */

#define NEED_AVX2 1

#include "Eina.h"
#include "Evas.h"
#include "evas_common_types.h"

EXPORTAPI void evas_common_cpu_end_opt(void);

#include "config.h"
#include "evas_blend_ops.h"

extern RGBA_Gfx_Func     op_copy_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
extern RGBA_Gfx_Func     op_copy_rel_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

# include "op_copy_pixel_avx2.c"
# include "op_copy_color_avx2.c"

void
evas_common_op_copy_init_avx2(void)
{
#ifdef BUILD_AVX2
   /* GA_MASK_AVX2 / RB_MASK_AVX2 are declared static in evas_blend_ops.h,
    * so - same CRITICAL note as op_blend_master_avx2.c - this TU gets its
    * own zero-initialised copies, separate from op_blend_master_avx2.c's.
    * mul_sym_avx2, used by the _rel kernels in this group, needs them set
    * here or every _rel call in this TU multiplies against zero masks.
    * Values copied verbatim from op_blend_master_avx2.c; keep in sync if
    * that file's ever change. */
   GA_MASK_AVX2 = _mm256_set1_epi32(0x00FF00FF);
   RB_MASK_AVX2 = _mm256_set1_epi32(0xFF00FF00);

   init_copy_pixel_span_funcs_avx2();
   init_copy_color_span_funcs_avx2();
#endif
}

void
evas_common_op_copy_rel_init_avx2(void)
{
#ifdef BUILD_AVX2
   init_copy_rel_pixel_span_funcs_avx2();
   init_copy_rel_color_span_funcs_avx2();
#endif
}
