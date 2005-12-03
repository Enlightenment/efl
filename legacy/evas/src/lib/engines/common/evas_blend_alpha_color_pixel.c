#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

// extern DATA8  *_evas_pow_lut;
extern const DATA8  _evas_pow_lut[65536];
extern const DATA16 _evas_const_c1[4];

#ifdef BUILD_C
void
evas_common_blend_alpha_color_rgba_to_rgba_c (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len, ca = A_VAL(&col);

   ca += ca >> 7;
   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
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
	     }
		break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		a = (a * ca) >> 8;
		switch(da)
		  {
		   case 0:
		      *dst = (col & 0x00ffffff) | (a << 24);
		      break;
		   case 255:
		      a += a >> 7;
		      *dst += RGB_JOIN( ((R_VAL(&col) - R_VAL(dst)) * a) >> 8,
		                        ((G_VAL(&col) - G_VAL(dst)) * a) >> 8,
		                        ((B_VAL(&col) - B_VAL(dst)) * a) >> 8 );
		      break;
		   default:
		      da = _evas_pow_lut[(a << 8) | da];
		      da += da >> 7;  a += a >> 7;
		      *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * a) >> 8,
		                         ((R_VAL(&col) - R_VAL(dst)) * da) >> 8,
		                         ((G_VAL(&col) - G_VAL(dst)) * da) >> 8,
		                         ((B_VAL(&col) - B_VAL(dst)) * da) >> 8 );
		      break;
		  }
	     }
		break;
	  }
     }
}

void
evas_common_blend_alpha_color_rgb_to_rgba_c (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		*dst = col;
		break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		      *dst = (col & 0x00ffffff) | (a << 24);
		      break;
		   case 255:
		      a += a >> 7;
		      *dst += RGB_JOIN( ((R_VAL(&col) - R_VAL(dst)) * a) >> 8,
		                        ((G_VAL(&col) - G_VAL(dst)) * a) >> 8,
		                        ((B_VAL(&col) - B_VAL(dst)) * a) >> 8 );
		      break;
		   default:
		      da = _evas_pow_lut[(a << 8) | da];
		      da += da >> 7;  a += a >> 7;
		      *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * a) >> 8,
		                         ((R_VAL(&col) - R_VAL(dst)) * da) >> 8,
		                         ((G_VAL(&col) - G_VAL(dst)) * da) >> 8,
		                         ((B_VAL(&col) - B_VAL(dst)) * da) >> 8 );
		      break;
		  }
	     }
		break;
	  }
     }
}

void
evas_common_blend_alpha_color_rgba_to_rgb_c (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len, ca = A_VAL(&col);

   ca += ca >> 7;
   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		*dst += RGB_JOIN( ((R_VAL(&col) - R_VAL(dst)) * ca) >> 8,
		                  ((G_VAL(&col) - G_VAL(dst)) * ca) >> 8,
		                  ((B_VAL(&col) - B_VAL(dst)) * ca) >> 8 );
		break;
	   default:
		a = (a * ca) >> 8;
		a += a >> 7;
		*dst += RGB_JOIN( ((R_VAL(&col) - R_VAL(dst)) * a) >> 8,
		                  ((G_VAL(&col) - G_VAL(dst)) * a) >> 8,
		                  ((B_VAL(&col) - B_VAL(dst)) * a) >> 8 );
		break;
	  }
     }
}

void
evas_common_blend_alpha_color_rgb_to_rgb_c (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		*dst = (*dst | 0x00ffffff) & col;
		break;
	   default:
		a += a >> 7;
		*dst += RGB_JOIN( ((R_VAL(&col) - R_VAL(dst)) * a) >> 8,
		                  ((G_VAL(&col) - G_VAL(dst)) * a) >> 8,
		                  ((B_VAL(&col) - B_VAL(dst)) * a) >> 8 );
		break;
	  }
     }
}
#endif

/* ************************************************************************** */

#ifdef BUILD_MMX
void
evas_common_blend_alpha_color_rgba_to_rgba_mmx (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

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

   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
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
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      da = _evas_pow_lut[(A_VAL(&col) << 8) + da];
		      da = ARGB_JOIN(A_VAL(&col), da, da, da);
		      movd_m2r(da, mm3);
		      punpcklbw_r2r(mm3, mm3);   // mm3 = [AA][dAdA][dAdA][dAdA]
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
	     }
		break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		a = RGB_JOIN(a,a,a);
		movd_m2r(a, mm3);
		punpcklbw_r2r(mm0, mm3);
		psllw_i2r(1, mm3);
		paddw_r2r(mm6, mm3);
		pmulhw_r2r(mm5, mm3);
		
		packuswb_r2r(mm0, mm3);
		movd_r2m(mm3, a);
		a &= 0xff;
		punpcklbw_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);
		
		switch(da)
		  {
		    case 0:
		      *dst = (col & 0x00ffffff) | (a << 24);
		      break;
		    case 255:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      movq_r2r(mm1, mm4);
		      psubw_r2r(mm2, mm4);
		      psllw_i2r(1, mm4);
		      paddw_r2r(mm6, mm4);
	
		      pmulhw_r2r(mm3, mm4);
		      paddw_r2r(mm4, mm2);
		      
		      packuswb_r2r(mm0, mm2);
		      movd_r2m(mm2, *dst);

		      break;
		    default:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      da = _evas_pow_lut[(a << 8) + da];
		      da = ARGB_JOIN(a, da, da, da);
		      movd_m2r(da, mm3);
		      punpcklbw_r2r(mm3, mm3);   // mm3 = [AA][dAdA][dAdA][dAdA]
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
	     }
		break;
	  }
     }
}

void
evas_common_blend_alpha_color_rgb_to_rgba_mmx (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   movd_m2r(col, mm1);
   punpcklbw_r2r(mm0, mm1);

   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		*dst = col;
		break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		      *dst = (col & 0x00ffffff) | (a << 24);
		      break;
		   case 255:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      a = a << 24;
		      movd_m2r(a, mm3);
		      punpcklbw_r2r(mm3, mm3);
		      punpckhwd_r2r(mm3, mm3);
		      punpckhdq_r2r(mm3, mm3);
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
		   default:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      da = _evas_pow_lut[(a << 8) + da];
		      da = ARGB_JOIN(a, da, da, da);
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
	     }
		break;
	  }
     }
}

void
evas_common_blend_alpha_color_rgba_to_rgb_mmx (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

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
   
   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);
	
		movq_r2r(mm1, mm3);
	        psubw_r2r(mm2, mm3);    // mm3 = [A-a][R-r][G-g][B-b]
	        psllw_i2r(1, mm3);      // mm3 = [A*2][R*2][G*2][B*2]
	        paddw_r2r(mm6, mm3);    // mm3 = [A+1][R+1][G+1][B+1]
	        pmulhw_r2r(mm5, mm3);   // mm3 = [A*0][(R*AA)>>16][(G*AA)>>16][(B*AA)>>16]
	        paddw_r2r(mm3, mm2);
	
		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	   default:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);
		
		a = RGB_JOIN(a,a,a);
		movd_m2r(a, mm3);
		punpcklbw_r2r(mm0, mm3);
	        psllw_i2r(1, mm3);
		paddw_r2r(mm6, mm3);
		pmulhw_r2r(mm5, mm3);
		
		packuswb_r2r(mm0, mm3);
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
     }
}

void
evas_common_blend_alpha_color_rgb_to_rgb_mmx (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   col &= 0x00ffffff;
   movd_m2r(col, mm1);
   punpcklbw_r2r(mm0, mm1);

   mask--; dst--;
   while (++mask, ++dst < dst_end)
     {
	DATA32  a = *mask;

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		*dst = (*dst & 0xff000000) | col;
		break;
	   default:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		a = a << 24;
		movd_m2r(a, mm3);
		punpcklbw_r2r(mm3, mm3);
		punpckhwd_r2r(mm3, mm3);
		punpckhdq_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);
//		psrlq_i2r(16, mm3);

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
     }
}
#endif

/* ************************************************************************** */

static void
evas_common_blend_alpha_color_nothing (DATA8 *mask, DATA32 *dst, int len, DATA32 col)
{
}
/* ************************************************************************** */

#ifdef BUILD_MMX
static Gfx_Func_Blend_Src_Alpha_Mul_Dst
evas_common_gfx_func_blend_alpha_col_get_mmx(DATA32 col, RGBA_Image *dst)
{
   if (A_VAL(&col) < 255)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	   evas_common_blend_init_evas_pow_lut();
	   return evas_common_blend_alpha_color_rgba_to_rgba_mmx;
	  }
	return evas_common_blend_alpha_color_rgba_to_rgb_mmx;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
     {
//	evas_common_blend_init_evas_pow_lut();
	return evas_common_blend_alpha_color_rgb_to_rgba_mmx;
     }
   return evas_common_blend_alpha_color_rgb_to_rgb_mmx;
}
#endif

#ifdef BUILD_C
static Gfx_Func_Blend_Src_Alpha_Mul_Dst
evas_common_gfx_func_blend_alpha_col_get_c(DATA32 col, RGBA_Image *dst)
{
   if (A_VAL(&col) < 255)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	   evas_common_blend_init_evas_pow_lut();
	   return evas_common_blend_alpha_color_rgba_to_rgba_c;
	  }
	return evas_common_blend_alpha_color_rgba_to_rgb_c;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
     {
//	evas_common_blend_init_evas_pow_lut();
	return evas_common_blend_alpha_color_rgb_to_rgba_c;
     }
   return evas_common_blend_alpha_color_rgb_to_rgb_c;
}
#endif

Gfx_Func_Blend_Src_Alpha_Mul_Dst
evas_common_draw_func_blend_alpha_get (DATA32 col, RGBA_Image *dst)
{
   Gfx_Func_Blend_Src_Alpha_Mul_Dst  func = NULL;

#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	func = evas_common_gfx_func_blend_alpha_col_get_mmx(col, dst);
	if (func)  return func;
     }
#endif
#ifdef BUILD_C
	func = evas_common_gfx_func_blend_alpha_col_get_c(col, dst);
	if (func)  return func;
#endif
   return evas_common_blend_alpha_color_nothing;
}

