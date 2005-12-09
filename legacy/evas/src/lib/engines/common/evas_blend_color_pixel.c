#include "evas_common.h"

#if defined BUILD_MMX || defined BUILD_SSE
#include "evas_mmx.h"
#endif

// extern DATA8       *_evas_pow_lut;
extern const DATA8 _evas_pow_lut[65536];
extern const DATA16 _evas_const_c1[4];

#ifdef BUILD_C
void
evas_common_blend_color_rgba_to_rgba_c(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len, ca = A_VAL(&col);

   ca += ca >> 7;
   while (dst < dst_end)
     {
	DATA32  da;
	
	da = A_VAL(dst);
	switch (da)
	  {
	   case 0:
	     *dst = col;
	     break;
	   case 255:
	     *dst += RGB_JOIN( ((R_VAL(&col) - R_VAL(dst)) * ca) >> 8,
			       ((G_VAL(&col) - G_VAL(dst)) * ca) >> 8,
			       ((B_VAL(&col) - B_VAL(dst)) * ca) >> 8 );
	     break;
	   default:
	     da = _evas_pow_lut[(A_VAL(&col) << 8) | da];
	     da += da >> 7;
	     *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * ca) >> 8,
				((R_VAL(&col) - R_VAL(dst)) * da) >> 8,
				((G_VAL(&col) - G_VAL(dst)) * da) >> 8,
				((B_VAL(&col) - B_VAL(dst)) * da) >> 8 );
	     break;
	  }
	dst++;
     }
}

void
evas_common_blend_color_rgba_to_rgb_c(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_end, ca;
   
   dst_end = dst + len;
   ca = A_VAL(&col);
   ca += ca >> 7;
   while (dst < dst_end)
     {
	*dst += RGB_JOIN(((R_VAL(&col) - R_VAL(dst)) * ca) >> 8,
			 ((G_VAL(&col) - G_VAL(dst)) * ca) >> 8,
			 ((B_VAL(&col) - B_VAL(dst)) * ca) >> 8 );
	dst++;
     }
}

void
evas_common_copy_color_rgba_to_rgba_c(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_end;
   
   dst_end = dst + len;
   while (dst < dst_end)
     {
	*dst = col;
	dst++;
     }
}

void
evas_common_copy_color_rgb_to_rgba_c(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_end;
   
   dst_end = dst + len;
   col |= PIXEL_SOLID_ALPHA;
   while (dst < dst_end)
     {
	*dst = col;
	dst++;
     }
}

void
evas_common_copy_color_rgb_to_rgb_c(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_end;

   dst_end = dst + len;
   while (dst < dst_end)
     {
	*dst = col & (*dst | 0x00ffffff);
	dst++;
     }
}
#endif

/****************************************************************************/

#ifdef BUILD_MMX
void
evas_common_blend_color_rgba_to_rgba_mmx(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_end;
   
   dst_end = dst + len;
   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   movd_m2r(col, mm3);
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   punpckhwd_r2r(mm3, mm3);
   punpckhdq_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);	

   movq_r2r(mm3, mm5);

   len = A_VAL(&col);
   col |= 0xff000000;
   movd_m2r(col, mm1);
   punpcklbw_r2r(mm0, mm1);
   A_VAL(&col) = len;

   while (dst < dst_end)
     {
	DATA32  da = A_VAL(dst);

	switch(da)
	  {
	   case 0:
	     *dst = col;
	     break;
	   case 255:
	     movd_m2r(*dst, mm2);
	     punpcklbw_r2r(mm0, mm2);
	     
	     movq_r2r(mm1, mm3);
	     psubw_r2r(mm2, mm3);
	     psllw_i2r(1, mm3);
	     paddw_r2r(mm6, mm3);
	     pmulhw_r2r(mm5, mm3);
	     paddw_r2r(mm3, mm2);
	     
	     packuswb_r2r(mm0, mm2);
	     movd_r2m(mm2, *dst);
	     
	     break;
	   default:
	     da = _evas_pow_lut[(A_VAL(&col) << 8) + da];
	     da = ARGB_JOIN(A_VAL(&col), da, da, da);
	     movd_m2r(da, mm3);
	     punpcklbw_r2r(mm3, mm3);
	     psrlw_i2r(1, mm3);
	     
	     movq_r2r(mm1, mm4);
	     psubw_r2r(mm2, mm4);
	     psllw_i2r(1, mm4);
	     paddw_r2r(mm6, mm4);
	     
	     pmulhw_r2r(mm3, mm4);
	     paddw_r2r(mm4, mm2);
	     
	     packuswb_r2r(mm0, mm2);
	     movd_r2m(mm2, *dst);
	     
	     break;
	  }
	dst++;
     }
}

void
evas_common_blend_color_rgba_to_rgb_mmx(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_end;
   
   dst_end = dst + len;
   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   punpckhwd_r2r(mm3, mm3);
   punpckhdq_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);	

   movq_r2r(mm3, mm5);

   col |= 0xff000000;
   movd_m2r(col, mm1);
   punpcklbw_r2r(mm0, mm1);

   while (dst < dst_end)
     {
	movd_m2r(*dst, mm2);
	punpcklbw_r2r(mm0, mm2);
	
	movq_r2r(mm1, mm3);
	psubw_r2r(mm2, mm3);
	psllw_i2r(1, mm3);
	paddw_r2r(mm6, mm3);
	pmulhw_r2r(mm5, mm3);
	paddw_r2r(mm3, mm2);
	
	packuswb_r2r(mm0, mm2);
	movd_r2m(mm2, *dst);
	dst++;
     }
}

void
evas_common_copy_color_rgba_to_rgba_mmx(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   dst_end_ptr_pre = dst + ((len / 10) * 10);

   movd_m2r(col, mm0);
   movd_m2r(col, mm1);
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
	*dst_ptr = col;
	dst_ptr++;
     }
}

#define evas_common_copy_color_rgb_to_rgb_mmx  evas_common_copy_color_rgba_to_rgba_mmx
#define evas_common_copy_color_rgb_to_rgba_mmx  evas_common_copy_color_rgba_to_rgba_mmx
#endif

/****************************************************************************/

#ifdef BUILD_SSE
void
evas_common_copy_color_rgba_to_rgba_sse(DATA32 col, DATA32 *dst, int len)
{
   DATA32 *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   dst_end_ptr_pre = dst + ((len / 10) * 10);

   movd_m2r(col, mm0);
   movd_m2r(col, mm1);
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
	*dst_ptr = col;
	dst_ptr++;
     }
}

#define evas_common_blend_color_rgba_to_rgba_sse  NULL
#define evas_common_blend_color_rgba_to_rgb_sse  NULL
#define evas_common_copy_color_rgb_to_rgb_sse  evas_common_copy_color_rgba_to_rgba_sse
#define evas_common_copy_color_rgb_to_rgba_sse  evas_common_copy_color_rgba_to_rgba_sse
#endif

/****************************************************************************/

static void
evas_common_blend_color_nothing(DATA32 col, DATA32 *dst, int len)
{
}

/****************************************************************************/

#ifdef BUILD_SSE
static Gfx_Func_Blend_Color_Dst
evas_common_gfx_func_blend_col_get_sse(DATA32 col, RGBA_Image *dst, int pixels)
{
   if (A_VAL(&col) < 255)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	   return evas_common_blend_color_rgba_to_rgba_sse;
	return evas_common_blend_color_rgba_to_rgb_sse;
     }
   if (pixels <= 65536) return NULL;
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_color_rgb_to_rgba_sse;
   return evas_common_copy_color_rgb_to_rgb_sse;
}
#endif

#ifdef BUILD_MMX
static Gfx_Func_Blend_Color_Dst
evas_common_gfx_func_blend_col_get_mmx(DATA32 col, RGBA_Image *dst, int pixels)
{
   if (A_VAL(&col) < 255)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	   evas_common_blend_init_evas_pow_lut();
	     return evas_common_blend_color_rgba_to_rgba_mmx;
	  }
	return evas_common_blend_color_rgba_to_rgb_mmx;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_color_rgb_to_rgba_mmx;
   return evas_common_copy_color_rgb_to_rgb_mmx;
}
#endif

#ifdef BUILD_C
static Gfx_Func_Blend_Color_Dst
evas_common_gfx_func_blend_col_get_c(DATA32 col, RGBA_Image *dst, int pixels)
{
   if (A_VAL(&col) < 255)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	   evas_common_blend_init_evas_pow_lut();
	   return evas_common_blend_color_rgba_to_rgba_c;
	  }
	return evas_common_blend_color_rgba_to_rgb_c;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_color_rgb_to_rgba_c;
   return evas_common_copy_color_rgb_to_rgb_c;
}
#endif

Gfx_Func_Blend_Color_Dst
evas_common_draw_func_blend_color_get (DATA32 col, RGBA_Image *dst, int pixels)
{
   Gfx_Func_Blend_Color_Dst  func = NULL;

#ifdef BUILD_SSE
   if (evas_common_cpu_has_feature(CPU_FEATURE_SSE))
     {
	func = evas_common_gfx_func_blend_col_get_sse(col, dst, pixels);
	if (func)  return func;
     }
#endif
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	func = evas_common_gfx_func_blend_col_get_mmx(col, dst, pixels);
	if (func)  return func;
     }
#endif
#ifdef BUILD_C
   func = evas_common_gfx_func_blend_col_get_c(col, dst, pixels);
   if (func)  return func;
#endif
   return evas_common_blend_color_nothing;
}

