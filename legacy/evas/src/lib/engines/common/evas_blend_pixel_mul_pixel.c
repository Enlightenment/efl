#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

extern DATA8        _evas_pow_lut[256][256];
extern const DATA16 _evas_const_c1[4];

#ifdef BUILD_C
void
evas_common_blend_pixels_mul_color_rgba_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA32 mul_color)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   if ((R_VAL(&mul_color) == 255) && 
       (G_VAL(&mul_color) == 255) &&
       (B_VAL(&mul_color) == 255))
     {
	while (dst_ptr < dst_end_ptr)
	  {
	     DATA32 tmp;
	     DATA8  a;
	     
	     a = (A_VAL(src_ptr) * (A_VAL(&mul_color) + 1)) >> 8;
	     switch (a)
	       {
		case 0:
		  break;
		case 255:
		  *dst_ptr = *src_ptr;
		  break;
		default:
		  BLEND_COLOR(a, R_VAL(dst_ptr), 
			      R_VAL(src_ptr), R_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, G_VAL(dst_ptr), 
			      G_VAL(src_ptr), G_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, B_VAL(dst_ptr), 
			      B_VAL(src_ptr), B_VAL(dst_ptr), 
			      tmp);
		  break;
	       }
	     src_ptr++;
	     dst_ptr++;
	  }
     }
   else
     {
	while (dst_ptr < dst_end_ptr)
	  {
	     DATA32 tmp;
	     DATA8  a;
	     
	     a = (A_VAL(src_ptr) * (A_VAL(&mul_color) + 1)) >> 8;
	     switch (a)
	       {
		case 0:
		  break;
		case 255:
		  R_VAL(dst_ptr) = ((R_VAL(src_ptr) * (R_VAL(&mul_color) + 1)) >> 8);
		  G_VAL(dst_ptr) = ((G_VAL(src_ptr) * (G_VAL(&mul_color) + 1)) >> 8);
		  B_VAL(dst_ptr) = ((B_VAL(src_ptr) * (B_VAL(&mul_color) + 1)) >> 8);
		  break;
		default:
		  BLEND_COLOR(a, R_VAL(dst_ptr), 
			      ((R_VAL(src_ptr) * (R_VAL(&mul_color) + 1)) >> 8), R_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, G_VAL(dst_ptr), 
			      ((G_VAL(src_ptr) * (G_VAL(&mul_color) + 1)) >> 8), G_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, B_VAL(dst_ptr), 
			      ((B_VAL(src_ptr) * (B_VAL(&mul_color) + 1)) >> 8), B_VAL(dst_ptr), 
			      tmp);
		  break;
	       }
	     src_ptr++;
	     dst_ptr++;
	  }
     }
}
#endif

#ifdef BUILD_MMX
void
evas_common_blend_pixels_mul_color_rgba_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 mul_color)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   pxor_r2r(mm6, mm6);
   movd_m2r(mul_color, mm1);
   punpcklbw_r2r(mm1, mm6);
   psrlw_i2r(8, mm6);
   movq_m2r(*_evas_const_c1, mm5);
   paddw_r2r(mm5, mm6);
   
   pxor_r2r(mm4, mm4);
   
   while (dst_ptr < dst_end_ptr)
     {
	DATA8 a;
	
	a = (A_VAL(src_ptr) * (A_VAL(&mul_color) + 1)) >> 8;
	switch (a)
	  {
	   case 0:
	     break;
	   case 255:
	     movd_m2r(src_ptr[0], mm1);

	     /* this could be more optimial.. but it beats the c code by almost */
	     /* double */
	     pxor_r2r(mm7, mm7);
	     punpcklbw_r2r(mm1, mm7);
	     psrlw_i2r(8, mm7);
	     pmullw_r2r(mm6, mm7);
	     psrlw_i2r(8, mm7);
	     packuswb_r2r(mm7, mm7);
	     movq_r2r(mm7, mm1);
	     movd_r2m(mm1, dst_ptr[0]);
	     break;
	   default:
	     movd_m2r(src_ptr[0], mm1);

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

/****************************************************************************/

void
evas_common_blend_pixels_mul_color_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA32 mul_color)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   if ((R_VAL(&mul_color) == 255) &&
       (G_VAL(&mul_color) == 255) &&
       (B_VAL(&mul_color) == 255))
     {
	while (dst_ptr < dst_end_ptr)
	  {
	     DATA32 tmp;
	     DATA8  a, aa;
	     
	     aa = (A_VAL(src_ptr) * (A_VAL(&mul_color) + 1)) >> 8;
	     switch (aa)
	       {
		case 0:
		  break;
		case 255:
		  *dst_ptr = *src_ptr;
		  break;
		default:
		  a = _evas_pow_lut[aa][A_VAL(dst_ptr)];
		  
		  BLEND_COLOR(a, R_VAL(dst_ptr), 
			      R_VAL(src_ptr), R_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, G_VAL(dst_ptr), 
			      G_VAL(src_ptr), G_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, B_VAL(dst_ptr), 
			      B_VAL(src_ptr), B_VAL(dst_ptr), 
			      tmp);
		  A_VAL(dst_ptr) = A_VAL(dst_ptr) + ((aa * (255 - A_VAL(dst_ptr))) / 255);
		  break;
	       }
	     src_ptr++;
	     dst_ptr++;
	  }
     }
   else
     {
	while (dst_ptr < dst_end_ptr)
	  {
	     DATA32 tmp;
	     DATA8  a, aa;
	     
	     aa = (A_VAL(src_ptr) * (A_VAL(&mul_color) + 1)) >> 8;
	     switch (aa)
	       {
		case 0:
		  break;
		case 255:
		  R_VAL(dst_ptr) = ((R_VAL(src_ptr) * (R_VAL(&mul_color) + 1)) >> 8);
		  G_VAL(dst_ptr) = ((G_VAL(src_ptr) * (G_VAL(&mul_color) + 1)) >> 8);
		  B_VAL(dst_ptr) = ((B_VAL(src_ptr) * (B_VAL(&mul_color) + 1)) >> 8);
		  A_VAL(dst_ptr) = 255;
		  break;
		default:
		  a = _evas_pow_lut[aa][A_VAL(dst_ptr)];
		  
		  BLEND_COLOR(a, R_VAL(dst_ptr), 
			      ((R_VAL(src_ptr) * (R_VAL(&mul_color) + 1)) >> 8), R_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, G_VAL(dst_ptr), 
			      ((G_VAL(src_ptr) * (G_VAL(&mul_color) + 1)) >> 8), G_VAL(dst_ptr), 
			      tmp);
		  BLEND_COLOR(a, B_VAL(dst_ptr), 
			      ((B_VAL(src_ptr) * (B_VAL(&mul_color) + 1)) >> 8), B_VAL(dst_ptr), 
			      tmp);
		  A_VAL(dst_ptr) = A_VAL(dst_ptr) + ((aa * (255 - A_VAL(dst_ptr))) / 255);
		  break;
	       }
	     src_ptr++;
	     dst_ptr++;
	  }
     }
}
