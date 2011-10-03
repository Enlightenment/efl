#define NEED_SSE3 1

#include "evas_common.h"

#ifdef BUILD_SSE3
static __m128i A_MASK_SSE3;
#endif

extern RGBA_Gfx_Func     op_blend_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
extern RGBA_Gfx_Pt_Func  op_blend_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

extern RGBA_Gfx_Func     op_blend_rel_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
extern RGBA_Gfx_Pt_Func  op_blend_rel_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

# include "op_blend_pixel_sse3.c"
# include "op_blend_color_sse3.c"
# include "op_blend_pixel_color_sse3.c"
# include "op_blend_pixel_mask_sse3.c"
# include "op_blend_mask_color_sse3.c"

void
evas_common_op_blend_init_sse3(void)
{
#ifdef BUILD_SSE3
   GA_MASK_SSE3 = _mm_set_epi32(0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF);
   RB_MASK_SSE3 = _mm_set_epi32(0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00);
   SYM4_MASK_SSE3 = _mm_set_epi32(0x00FF00FF, 0x000000FF, 0x00FF00FF, 0x000000FF);
   RGB_MASK_SSE3 = _mm_set_epi32(0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF);
   A_MASK_SSE3 = _mm_set_epi32(0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000);
   ALPHA_SSE3 = _mm_set_epi32(256, 256, 256, 256);

   init_blend_pixel_span_funcs_sse3();
   init_blend_pixel_color_span_funcs_sse3();
   init_blend_pixel_mask_span_funcs_sse3();
   init_blend_color_span_funcs_sse3();
   init_blend_mask_color_span_funcs_sse3();

   init_blend_pixel_pt_funcs_sse3();
   init_blend_pixel_color_pt_funcs_sse3();
   init_blend_pixel_mask_pt_funcs_sse3();
   init_blend_color_pt_funcs_sse3();
   init_blend_mask_color_pt_funcs_sse3();
#endif   
}

void
evas_common_op_blend_rel_init_sse3(void)
{
#ifdef BUILD_SSE3
   init_blend_rel_pixel_span_funcs_sse3();
   init_blend_rel_pixel_color_span_funcs_sse3();
   init_blend_rel_pixel_mask_span_funcs_sse3();
   init_blend_rel_color_span_funcs_sse3();
   init_blend_rel_mask_color_span_funcs_sse3();

   init_blend_rel_pixel_pt_funcs_sse3();
   init_blend_rel_pixel_color_pt_funcs_sse3();
   init_blend_rel_pixel_mask_pt_funcs_sse3();
   init_blend_rel_color_pt_funcs_sse3();
   init_blend_rel_mask_color_pt_funcs_sse3();
#endif   
}

//#pragma GCC push_options
//#pragma GCC optimize ("O0")
void
evas_common_op_sse3_test(void)
{
#ifdef BUILD_SSE3
   DATA32 s[64] = {0x11883399}, d[64] = {0xff88cc33};
   
   s[0] = rand(); d[1] = rand();
   _op_blend_pas_dp_sse3(s, NULL, 0, d, 64);
   evas_common_cpu_end_opt();
#endif   
}
//#pragma GCC pop_options
