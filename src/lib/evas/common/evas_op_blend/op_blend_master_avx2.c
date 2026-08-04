/* AVX2 blend kernels.
 *
 * This translation unit is the only one in evas compiled with -mavx2, and it
 * is a separate static library for that reason: the SSE3 kernels next door run
 * on any SSE3 cpu, so letting the compiler emit AVX2 into them would fault on
 * hardware that has no AVX2. Whether the code in *this* file runs at all is
 * decided at runtime by CPU_FEATURE_AVX2.
 */

#define NEED_AVX2 1

#include "Eina.h"
#include "Evas.h"
#include "evas_common_types.h"

EXPORTAPI void evas_common_cpu_end_opt(void);

#include "config.h"
#include "evas_blend_ops.h"

extern RGBA_Gfx_Func     op_blend_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
extern RGBA_Gfx_Func     op_blend_rel_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

# include "op_blend_pixel_avx2.c"
# include "op_blend_color_avx2.c"

void
evas_common_op_blend_init_avx2(void)
{
#ifdef BUILD_AVX2
   GA_MASK_AVX2 = _mm256_set1_epi32(0x00FF00FF);
   RB_MASK_AVX2 = _mm256_set1_epi32(0xFF00FF00);
   ALPHA_AVX2   = _mm256_set1_epi32(256);

   init_blend_pixel_span_funcs_avx2();
   init_blend_color_span_funcs_avx2();
#endif
}

void
evas_common_op_blend_rel_init_avx2(void)
{
#ifdef BUILD_AVX2
#endif
}
