#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

extern DATA8        _evas_pow_lut[256][256];
extern const DATA16 _evas_const_c1[4];

#ifdef BUILD_C
void
evas_common_blend_alpha_color_rgba_to_rgb_c (DATA8 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA8 *src_ptr;
   DATA32 *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   
   if (A_VAL(&col) == 0) return;
   
   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8 aa;
	
	aa = (((*src_ptr) +1) * A_VAL(&col)) >> 8;
	switch (aa)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst_ptr = col;
	     break;
	   default:
	     BLEND_COLOR(aa, R_VAL(dst_ptr), 
			 R_VAL(&col), R_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(aa, G_VAL(dst_ptr), 
			 G_VAL(&col), G_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(aa, B_VAL(dst_ptr), 
			 B_VAL(&col), B_VAL(dst_ptr), 
			 tmp);
	     break;
	  }
	src_ptr++;
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_MMX
void
evas_common_blend_alpha_color_rgba_to_rgb_mmx (DATA8 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA8 *src_ptr;
   DATA32 *dst_ptr, *dst_end_ptr;
   
   if (A_VAL(&col) == 0) return;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   pxor_r2r(mm6, mm6);
   movd_m2r(col, mm1);
   punpcklbw_r2r(mm1, mm6);
   psrlw_i2r(8, mm6);
   movq_m2r(*_evas_const_c1, mm5);
   paddw_r2r(mm5, mm6);
   
   pxor_r2r(mm4, mm4);
   
   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
        DATA8 aa;
	
	aa = (((*src_ptr) +1) * A_VAL(&col)) >> 8;
	switch (aa)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst_ptr = col;
	     break;
	   default:
	     tmp = 0xffffff;
	     A_VAL(&tmp) = *src_ptr;
	
	     movd_m2r(tmp, mm1);

	     /* this could be more optimial.. but it beats the c code by almost */
	     /* double */
	     pxor_r2r(mm7, mm7);
	     punpcklbw_r2r(mm1, mm7);
	     psrlw_i2r(8, mm7);
	     pmullw_r2r(mm6, mm7);
	     psrlw_i2r(8, mm7);
	     packuswb_r2r(mm7, mm7);
	     movq_r2r(mm7, mm1);
	     /* and back to our normal programming... */
	     movd_m2r(dst_ptr[0], mm2);
	     
	     movq_r2r(mm1, mm3);
	     
	     punpcklbw_r2r(mm3, mm3);
	     punpckhwd_r2r(mm3, mm3);
	     punpckhdq_r2r(mm3, mm3);	
	     psrlw_i2r(1, mm3);
	     
	     psrlq_i2r(16, mm3);
	     
	     punpcklbw_r2r(mm4, mm1);
	     punpcklbw_r2r(mm4, mm2);
	     
	     psubw_r2r(mm2, mm1);
	     psllw_i2r(1, mm1);
	     paddw_r2r(mm5, mm1);
	     pmulhw_r2r(mm3, mm1);
	     paddw_r2r(mm1, mm2);
	     
	     packuswb_r2r(mm4, mm2);
	     movd_r2m(mm2, dst_ptr[0]);
	     break;
	  }
	src_ptr++;
	dst_ptr++;
     }   
}
#endif

void
evas_common_blend_alpha_color_rgba_to_rgba_c (DATA8 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA8 *src_ptr;
   DATA32 *dst_ptr, *dst_end_ptr;
   
   if (A_VAL(&col) == 0) return;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8  a;
	DATA8 aa;
	
	aa = (((*src_ptr) + 1) * A_VAL(&col)) >> 8;
	a = _evas_pow_lut[aa][A_VAL(dst_ptr)];
        switch (a)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst_ptr = col;
	     break;
	   default:
	     BLEND_COLOR(a, R_VAL(dst_ptr), 
			 R_VAL(&col), R_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, G_VAL(dst_ptr), 
			 G_VAL(&col), G_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, B_VAL(dst_ptr), 
			 B_VAL(&col), B_VAL(dst_ptr), 
			 tmp);
	     A_VAL(dst_ptr) = A_VAL(dst_ptr) + ((aa * (255 - A_VAL(dst_ptr))) / 255);
	     break;
	  }
	src_ptr++;
	dst_ptr++;
     }
}
