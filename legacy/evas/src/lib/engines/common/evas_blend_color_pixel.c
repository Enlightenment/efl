#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

extern DATA8       *_evas_pow_lut;
extern const DATA16 _evas_const_c1[4];

#ifdef BUILD_C
void
evas_common_blend_color_rgba_to_rgb_c(DATA32 src, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	
	BLEND_ALPHA_SETUP(A_VAL(&src), tmp);
	BLEND_COLOR(A_VAL(&src), R_VAL(dst_ptr), 
		    R_VAL(&src), R_VAL(dst_ptr), 
		    tmp);
	BLEND_COLOR(A_VAL(&src), G_VAL(dst_ptr), 
		    G_VAL(&src), G_VAL(dst_ptr), 
		    tmp);
	BLEND_COLOR(A_VAL(&src), B_VAL(dst_ptr), 
		    B_VAL(&src), B_VAL(dst_ptr), 
		    tmp);
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_MMX
void
evas_common_blend_color_rgba_to_rgb_mmx(DATA32 src, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   
   pxor_r2r(mm4, mm4);
   movq_m2r(*_evas_const_c1, mm5);
   
   movd_m2r(src, mm6);
   
   movq_r2r(mm6, mm3);
   punpcklbw_r2r(mm3, mm3);
   punpckhwd_r2r(mm3, mm3);
   punpckhdq_r2r(mm3, mm3);	
   psrlw_i2r(1, mm3);
	
//   psrlq_i2r(16, mm3);
	
   while (dst_ptr < dst_end_ptr)
     {
	movq_r2r(mm6, mm1);
	
	movd_m2r(dst_ptr[0], mm2);
	
	punpcklbw_r2r(mm4, mm1);
	punpcklbw_r2r(mm4, mm2);
	
	psubw_r2r(mm2, mm1);
	psllw_i2r(1, mm1);
	paddw_r2r(mm5, mm1);
	pmulhw_r2r(mm3, mm1);
	paddw_r2r(mm1, mm2);
	
	packuswb_r2r(mm4, mm2);
	movd_r2m(mm2, dst_ptr[0]);
	
	dst_ptr++;
     }   
}
#endif

/****************************************************************************/

void
evas_common_blend_color_rgba_to_rgba_c(DATA32 src, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8  a;
	
	a = _evas_pow_lut[(A_VAL(&src) << 8) | A_VAL(dst_ptr)];
	
	BLEND_COLOR(A_VAL(&src), A_VAL(dst_ptr), 
		    255, A_VAL(dst_ptr), 
		    tmp);
	BLEND_ALPHA_SETUP(a, tmp);
	BLEND_COLOR(a, R_VAL(dst_ptr), 
		    R_VAL(&src), R_VAL(dst_ptr), 
		    tmp);
	BLEND_COLOR(a, G_VAL(dst_ptr), 
		    G_VAL(&src), G_VAL(dst_ptr), 
		    tmp);
	BLEND_COLOR(a, B_VAL(dst_ptr), 
		    B_VAL(&src), B_VAL(dst_ptr), 
		    tmp);	
	
	dst_ptr++;
     }
}

/****************************************************************************/

#ifdef BUILD_C
void
evas_common_copy_color_rgba_to_rgba_c(DATA32 src, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = src;
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_MMX
void
evas_common_copy_color_rgba_to_rgba_mmx(DATA32 src, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   dst_end_ptr_pre = dst + ((len / 10) * 10);

   movd_m2r(src, mm0);   
   movd_m2r(src, mm1);   
   psllq_i2r(32, mm0);
   por_r2r(mm1, mm0);
   
   while (dst_ptr < dst_end_ptr_pre)
     {
	movq_r2m(mm0, dst_ptr[0]);
	movq_r2m(mm0, dst_ptr[2]);
	movq_r2m(mm0, dst_ptr[4]);
	movq_r2m(mm0, dst_ptr[6]);
	movq_r2m(mm0, dst_ptr[8]);
	dst_ptr+=10;
     }
   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = src;
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_SSE
void
evas_common_copy_color_rgba_to_rgba_sse(DATA32 src, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   dst_end_ptr_pre = dst + ((len / 10) * 10);

   movd_m2r(src, mm0);   
   movd_m2r(src, mm1);   
   psllq_i2r(32, mm0);
   por_r2r(mm1, mm0);
   
   while (dst_ptr < dst_end_ptr_pre)
     {
	prefetch(&dst_ptr[128]);
	movq_r2m(mm0, dst_ptr[0]);
	movq_r2m(mm0, dst_ptr[2]);
	movq_r2m(mm0, dst_ptr[4]);
	movq_r2m(mm0, dst_ptr[6]);
	movq_r2m(mm0, dst_ptr[8]);
	dst_ptr+=10;
     }
   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = src;
	dst_ptr++;
     }
}
#endif

/****************************************************************************/

void
evas_common_copy_color_rgb_to_rgba_c(DATA32 src, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   src |= PIXEL_SOLID_ALPHA;
   
   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = src;
	dst_ptr++;
     }
}
