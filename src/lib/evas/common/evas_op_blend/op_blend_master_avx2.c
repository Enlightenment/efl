/* AVX2 blend kernels.
 *
 * This translation unit is the only one in evas compiled with -mavx2, and it
 * is a separate static library for that reason: the SSE3 kernels next door run
 * on any SSE3 cpu, so letting the compiler emit AVX2 into them would fault on
 * hardware that has no AVX2. Whether the code in *this* file runs at all is
 * decided at runtime by CPU_FEATURE_AVX2.
 */

#define NEED_AVX2 1

/* Also need the SSE3 helpers (mul4_sym_sse3, mul3_sym_sse3, mul_256_sse3,
 * sub4_alpha_sse3, interp4_256_sse3): the 4-wide stage in every kernel in
 * this TU runs the SSE3 kernel's A4OP body verbatim, so AVX2 and SSE3
 * execute literally the same instructions at 4-7 pixels rather than merely
 * equivalent ones (see LOOP_ALIGNED_U1_A4_A8_A16 in evas_blend_ops.h). */
#define NEED_SSE3 1

#include "Eina.h"
#include "Evas.h"
#include "evas_common_types.h"

EXPORTAPI void evas_common_cpu_end_opt(void);

#include "config.h"
#include "evas_blend_ops.h"

#ifdef BUILD_AVX2
static __m256i A_MASK_AVX2;
#endif

/* CRITICAL: the SSE3 statics (GA_MASK_SSE3, RB_MASK_SSE3, SYM4_MASK_SSE3,
 * RGB_MASK_SSE3, ALPHA_SSE3, A_MASK_SSE3) declared by evas_blend_ops.h and
 * below are per-translation-unit. op_blend_master_sse3.c initialises ITS
 * own copies in evas_common_op_blend_init_sse3() - that does NOT reach this
 * TU's separate, zero-initialised copies. If evas_common_op_blend_init_avx2()
 * below did not set them too, every 4-wide block in this TU would multiply
 * against zero masks and produce garbage. Values copied verbatim from
 * op_blend_master_sse3.c; keep them in sync if that file's ever change. */
#ifdef BUILD_SSE3
static __m128i A_MASK_SSE3;
#endif

extern RGBA_Gfx_Func     op_blend_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
extern RGBA_Gfx_Func     op_blend_rel_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

# include "op_blend_pixel_avx2.c"
# include "op_blend_color_avx2.c"
# include "op_blend_pixel_color_avx2.c"
# include "op_blend_pixel_mask_avx2.c"
# include "op_blend_mask_color_avx2.c"

void
evas_common_op_blend_init_avx2(void)
{
#ifdef BUILD_AVX2
   GA_MASK_AVX2 = _mm256_set1_epi32(0x00FF00FF);
   RB_MASK_AVX2 = _mm256_set1_epi32(0xFF00FF00);
   ALPHA_AVX2   = _mm256_set1_epi32(256);
   SYM4_MASK_AVX2 = _mm256_set_epi32(0x00FF00FF, 0x000000FF, 0x00FF00FF, 0x000000FF, 0x00FF00FF, 0x000000FF, 0x00FF00FF, 0x000000FF);
   RGB_MASK_AVX2 = _mm256_set1_epi32(0xFFFFFF);
   A_MASK_AVX2 = _mm256_set1_epi32(0xFF000000);

#ifdef BUILD_SSE3
   /* This TU's own copies of the SSE3 constants, for the 4-wide stage - see
    * the CRITICAL comment above. Values must match
    * evas_common_op_blend_init_sse3() in op_blend_master_sse3.c exactly. */
   GA_MASK_SSE3 = _mm_set_epi32(0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF);
   RB_MASK_SSE3 = _mm_set_epi32(0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00);
   SYM4_MASK_SSE3 = _mm_set_epi32(0x00FF00FF, 0x000000FF, 0x00FF00FF, 0x000000FF);
   RGB_MASK_SSE3 = _mm_set_epi32(0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF);
   A_MASK_SSE3 = _mm_set_epi32(0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000);
   ALPHA_SSE3 = _mm_set_epi32(256, 256, 256, 256);
#endif

   init_blend_pixel_span_funcs_avx2();
   init_blend_color_span_funcs_avx2();
   init_blend_pixel_color_span_funcs_avx2();
   init_blend_pixel_mask_span_funcs_avx2();
   init_blend_mask_color_span_funcs_avx2();
#endif
}

void
evas_common_op_blend_rel_init_avx2(void)
{
#ifdef BUILD_AVX2
   init_blend_rel_pixel_span_funcs_avx2();
   init_blend_rel_color_span_funcs_avx2();
   init_blend_rel_pixel_color_span_funcs_avx2();
   init_blend_rel_pixel_mask_span_funcs_avx2();
   init_blend_rel_mask_color_span_funcs_avx2();
#endif
}
