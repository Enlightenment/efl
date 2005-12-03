#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

// extern DATA8       *_evas_pow_lut;
extern const DATA8  _evas_pow_lut[65536];
extern const DATA16 _evas_const_c1[4];


#ifdef BUILD_C
void
evas_common_blend_pixels_rgba_mul_color_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);
   DATA32  ca = A_VAL(&col), cr = R_VAL(&col),
	   cg = G_VAL(&col), cb = B_VAL(&col);

   ca += ca >>7;  cr += cr >> 7;
   cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0: break;
	   case 255:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		      *dst = ARGB_JOIN( A_VAL(&col),
		                        (R_VAL(src) * cr) >> 8,
		                        (G_VAL(src) * cg) >> 8,
		                        (B_VAL(src) * cb) >> 8 );
		      break;
		   case 255:
		      *dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * ca) >> 8,
		                        ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * ca) >> 8,
		                        ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * ca) >> 8 );
		      break;
		   default:
		      da = _evas_pow_lut[(A_VAL(&col) << 8) | da];
		      da += da >> 7;
		      *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * ca) >> 8,
		                         ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * da) >> 8,
		                         ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * da) >> 8,
		                         ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * da) >> 8 );
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
		      *dst = ARGB_JOIN( a,
		                        (R_VAL(src) * cr) >> 8,
		                        (G_VAL(src) * cg) >> 8,
		                        (B_VAL(src) * cb) >> 8 );
		      break;
		   case 255:
		      a += a >> 7;
		      *dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * a) >> 8,
		                        ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * a) >> 8,
		                        ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * a) >> 8 );
		      break;
		   default:
		      da = _evas_pow_lut[(a << 8) | da];
		      da += da >> 7;  a += a >> 7;
		      *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * a) >> 8,
		                         ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * da) >> 8,
		                         ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * da) >> 8,
		                         ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * da) >> 8 );
		      break;
		  }
	     }
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgba_mul_color_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;
   DATA32  cr = R_VAL(&col), cg = G_VAL(&col), cb = B_VAL(&col);

   cr += cr >> 7; cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		*dst = ARGB_JOIN( 255,
		                  (R_VAL(src) * cr) >> 8,
		                  (G_VAL(src) * cg) >> 8,
		                  (B_VAL(src) * cb) >> 8 );
		break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		      *dst = ARGB_JOIN( a,
		                        (R_VAL(src) * cr) >> 8,
		                        (G_VAL(src) * cg) >> 8,
		                        (B_VAL(src) * cb) >> 8 );
		      break;
		   case 255:
		      a += a >> 7;
		      *dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * a) >> 8,
		                        ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * a) >> 8,
		                        ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * a) >> 8 );
		      break;
		   default:
		      da = _evas_pow_lut[(a << 8) | da];
		      da += da >> 7;  a += a >> 7;
		      *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * a) >> 8,
		                         ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * da) >> 8,
		                         ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * da) >> 8,
		                         ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * da) >> 8 );
		      break;
		  }
	     }
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgba_mul_color_a_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);
   DATA32  ca = A_VAL(&col);

   ca += ca >>7;
   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0: break;
	   case 255:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		      *dst = (*src & 0x00ffffff) | (A_VAL(&col) << 24);
		      break;
		   case 255:
		      *dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * ca) >> 8,
		                        ((G_VAL(src) - G_VAL(dst)) * ca) >> 8,
		                        ((B_VAL(src) - B_VAL(dst)) * ca) >> 8 );
		      break;
		   default:
		      da = _evas_pow_lut[(A_VAL(&col) << 8) | da];
		      da += da >> 7;
		      *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * ca) >> 8,
		                         ((R_VAL(src) - R_VAL(dst)) * da) >> 8,
		                         ((G_VAL(src) - G_VAL(dst)) * da) >> 8,
		                         ((B_VAL(src) - B_VAL(dst)) * da) >> 8 );
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
		      *dst = (*src & 0x00ffffff) | (a << 24);
		      break;
		   case 255:
		      a += a >> 7;
		      *dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * a) >> 8,
		                        ((G_VAL(src) - G_VAL(dst)) * a) >> 8,
		                        ((B_VAL(src) - B_VAL(dst)) * a) >> 8 );
		      break;
		   default:
		      da = _evas_pow_lut[(a << 8) | da];
		      da += da >> 7;  a += a >> 7;
		      *dst += ARGB_JOIN( ((255 - A_VAL(dst)) * a) >> 8,
		                         ((R_VAL(src) - R_VAL(dst)) * da) >> 8,
		                         ((G_VAL(src) - G_VAL(dst)) * da) >> 8,
		                         ((B_VAL(src) - B_VAL(dst)) * da) >> 8 );
		      break;
		  }
	     }
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgb_mul_color_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);
   DATA32  ca = A_VAL(&col), cr = R_VAL(&col),
	   cg = G_VAL(&col), cb = B_VAL(&col);

   ca += ca >>7;  cr += cr >> 7;
   cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  da = A_VAL(dst);

	switch (da)
	  {
	   case 0:
		*dst = ARGB_JOIN( A_VAL(&col),
		                  (R_VAL(src) * cr) >> 8,
		                  (G_VAL(src) * cg) >> 8,
		                  (B_VAL(src) * cb) >> 8 );
		break;
	   case 255:
		*dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * ca) >> 8,
		                  ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * ca) >> 8,
		                  ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * ca) >> 8 );
		break;
	   default:
		da = _evas_pow_lut[(A_VAL(&col) << 8) | da];
		da += da >> 7;
		*dst += ARGB_JOIN( ((255 - A_VAL(dst)) * ca) >> 8,
		                   ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * da) >> 8,
		                   ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * da) >> 8,
		                   ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * da) >> 8 );
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgb_mul_color_a_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;
   DATA32  ca = A_VAL(&col);

   ca += ca >> 7;
   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  da = A_VAL(dst);

	switch (da)
	  {
	   case 0:
		*dst = (*src & 0x00ffffff) | (A_VAL(&col) << 24);
		break;
	   case 255:
		*dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * ca) >> 8,
		                  ((G_VAL(src) - G_VAL(dst)) * ca) >> 8,
		                  ((B_VAL(src) - B_VAL(dst)) * ca) >> 8 );
		break;
	   default:
		da = _evas_pow_lut[(A_VAL(&col) << 8) | da];
		da += da >> 7;
		*dst += ARGB_JOIN( ((255 - A_VAL(dst)) * ca) >> 8,
		                   ((R_VAL(src) - R_VAL(dst)) * da) >> 8,
		                   ((G_VAL(src) - G_VAL(dst)) * da) >> 8,
		                   ((B_VAL(src) - B_VAL(dst)) * da) >> 8 );
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgba_mul_color_rgba_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);
   DATA32  ca = A_VAL(&col), cr = R_VAL(&col),
	   cg = G_VAL(&col), cb = B_VAL(&col);

   ca += ca >>7;  cr += cr >> 7;
   cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0: break;
	   case 255:
		*dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * ca) >> 8,
		                  ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * ca) >> 8,
		                  ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * ca) >> 8 );
		break;
	   default:
		a = (a * ca) >> 8;  a += a >> 7;
		*dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * a) >> 8,
		                  ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * a) >> 8,
		                  ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * a) >> 8 );
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgba_mul_color_rgb_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;
   DATA32  cr = R_VAL(&col), cg = G_VAL(&col), cb = B_VAL(&col);

   cr += cr >> 7; cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		*dst = (*dst & 0xff000000) | RGB_JOIN( (R_VAL(src) * cr) >> 8,
		                                       (G_VAL(src) * cg) >> 8,
		                                       (B_VAL(src) * cb) >> 8 );
		break;
	   default:
		a += a >> 7;
		*dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * a) >> 8,
		                  ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * a) >> 8,
		                  ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * a) >> 8 );
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgb_mul_color_rgba_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);
   DATA32  ca = A_VAL(&col), cr = R_VAL(&col),
	   cg = G_VAL(&col), cb = B_VAL(&col);

   ca += ca >>7;  cr += cr >> 7;
   cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while(++src, ++dst < dst_end)
	*dst += RGB_JOIN( ((((R_VAL(src) * cr) >> 8) - R_VAL(dst)) * ca) >> 8,
		          ((((G_VAL(src) * cg) >> 8) - G_VAL(dst)) * ca) >> 8,
		          ((((B_VAL(src) * cb) >> 8) - B_VAL(dst)) * ca) >> 8 );
}

void
evas_common_blend_pixels_rgba_mul_color_a_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);
   DATA32  ca = A_VAL(&col);

   ca += ca >>7;
   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0: break;
	   case 255:
		*dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * ca) >> 8,
		                  ((G_VAL(src) - G_VAL(dst)) * ca) >> 8,
		                  ((B_VAL(src) - B_VAL(dst)) * ca) >> 8 );
		break;
	   default:
		a = (a * ca) >> 8;  a += a >> 7;
		*dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * a) >> 8,
		                  ((G_VAL(src) - G_VAL(dst)) * a) >> 8,
		                  ((B_VAL(src) - B_VAL(dst)) * a) >> 8 );
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgb_mul_color_a_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;
   DATA32  ca = A_VAL(&col);

   ca += ca >> 7;
   src--; dst--;
   while (++src, ++dst < dst_end)
	*dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * ca) >> 8,
	                  ((G_VAL(src) - G_VAL(dst)) * ca) >> 8,
	                  ((B_VAL(src) - B_VAL(dst)) * ca) >> 8 );
}

void
evas_common_copy_pixels_rgb_mul_color_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;
   DATA32  cr = R_VAL(&col), cg = G_VAL(&col), cb = B_VAL(&col);

   cr += cr >> 7; cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while (++src, ++dst < dst_end)
	*dst = ARGB_JOIN( 255,
	                  (R_VAL(src) * cr) >> 8,
	                  (G_VAL(src) * cg) >> 8,
	                  (B_VAL(src) * cb) >> 8 );
}

void
evas_common_copy_pixels_rgb_mul_color_rgb_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;
   DATA32  cr = R_VAL(&col), cg = G_VAL(&col), cb = B_VAL(&col);

   cr += cr >> 7; cg += cg >> 7; cb += cb >> 7;
   src--; dst--;
   while (++src, ++dst < dst_end)
	*dst = (*dst & 0xff000000) | RGB_JOIN( (R_VAL(src) * cr) >> 8,
	                                       (G_VAL(src) * cg) >> 8,
	                                       (B_VAL(src) * cb) >> 8 );
}
#endif

/****************************************************************************/

#ifdef BUILD_MMX
void
evas_common_blend_pixels_rgba_mul_color_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   len = A_VAL(&col);
   col |= 0xff000000;
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm4);
   A_VAL(&col) = len;
   
  src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

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
		     movd_m2r(*src, mm1);
		     punpcklbw_r2r(mm0, mm1);
		     psllw_i2r(1, mm1);
		     paddw_r2r(mm6, mm1);
		     pmulhw_r2r(mm5, mm1);
		     packuswb_r2r(mm0, mm1);
		     movd_r2m(mm1, *dst);

		     break;
		   case 255:
		     movd_m2r(*dst, mm2);
		     punpcklbw_r2r(mm0, mm2);

		     movd_m2r(*src, mm1);
		     punpcklbw_r2r(mm0, mm1);
		     psllw_i2r(1, mm1);
		     paddw_r2r(mm6, mm1);
		     pmulhw_r2r(mm5, mm1);
		     packuswb_r2r(mm0, mm1);

		     movq_r2r(mm1, mm3);
		     punpcklbw_r2r(mm3, mm3);
		     punpckhwd_r2r(mm3, mm3);
		     punpckhdq_r2r(mm3, mm3);
		     psrlw_i2r(1, mm3);
		     psrlq_i2r(16, mm3);

		     punpcklbw_r2r(mm0, mm1);

		     psubw_r2r(mm2, mm1);
		     psllw_i2r(1, mm1);
		     paddw_r2r(mm6, mm1);
		     pmulhw_r2r(mm3, mm1);
		     paddw_r2r(mm1, mm2);
	
		     packuswb_r2r(mm0, mm2);
		     movd_r2m(mm2, *dst);

		     break;
		   default:
		     movd_m2r(*dst, mm2);
		     punpcklbw_r2r(mm0, mm2);

		     movd_m2r(*src, mm1);
		     punpcklbw_r2r(mm0, mm1);
		     psllw_i2r(1, mm1);
		     paddw_r2r(mm6, mm1);
		     pmulhw_r2r(mm4, mm1);
		     packuswb_r2r(mm0, mm1);

		     da = _evas_pow_lut[(A_VAL(&col) << 8) + da];
		     da = ARGB_JOIN(A_VAL(&col), da, da, da);
		     movd_m2r(da, mm3);
		     punpcklbw_r2r(mm3, mm3);   // mm3 = [AA][dAdA][dAdA][dAdA]
		     psrlw_i2r(1, mm3);         // mm3 = [AA/2][dAdA/2][dAdA/2][dAdA/2]

		     psubw_r2r(mm2, mm1);
		     psllw_i2r(1, mm1);
		     paddw_r2r(mm6, mm1);
		     pmulhw_r2r(mm3, mm1);
		     paddw_r2r(mm1, mm2);
		      
		     packuswb_r2r(mm0, mm2);
		     movd_r2m(mm2, *dst);

		     break;
		  }
	     }
		break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		    movd_m2r(*src, mm1);
		    punpcklbw_r2r(mm0, mm1);
		    psllw_i2r(1, mm1);
		    paddw_r2r(mm6, mm1);
		    pmulhw_r2r(mm5, mm1);
		    packuswb_r2r(mm0, mm1);

		switch(da)
		  {
		    case 0:
		      movd_r2m(mm1, *dst);

		      break;
		    case 255:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      movq_r2r(mm1, mm3);
		      punpcklbw_r2r(mm3, mm3);
		      punpckhwd_r2r(mm3, mm3);
		      punpckhdq_r2r(mm3, mm3);
		      psrlw_i2r(1, mm3);
		      psrlq_i2r(16, mm3);

		      punpcklbw_r2r(mm0, mm1);
		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);
	
		      packuswb_r2r(mm0, mm2);
		      movd_r2m(mm2, *dst);

		      break;
		    default:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      movd_r2m(mm1, a);
		      da = _evas_pow_lut[(A_VAL(&a) << 8) + da];
		      da = ARGB_JOIN(A_VAL(&a), da, da, da);
		      movd_m2r(da, mm3);
		      punpcklbw_r2r(mm3, mm3);
		      psrlw_i2r(1, mm3);    // mm3 = [AA/2][dAdA/2][dAdA/2][dAdA/2]

		      a |= 0xff000000;
		      movd_m2r(a, mm1);     // mm1 = [  ][  ][255R][GB] (COL*SRC)
		      punpcklbw_r2r(mm0, mm1);

		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);

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
evas_common_blend_pixels_rgba_mul_color_rgb_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   col |= 0xff000000;
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:
		break;
	   case 255:
		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);
		movd_r2m(mm1, *dst);

		break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		switch(da)
		  {
		   case 0:
		      movd_r2m(mm1, *dst);

		      break;
		   case 255:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      movq_r2r(mm1, mm3);
		      punpcklbw_r2r(mm3, mm3);
		      punpckhwd_r2r(mm3, mm3);
		      punpckhdq_r2r(mm3, mm3);
		      psrlw_i2r(1, mm3);
		      psrlq_i2r(16, mm3);

		      punpcklbw_r2r(mm0, mm1);
		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);
	
		      packuswb_r2r(mm0, mm2);
		      movd_r2m(mm2, *dst);

		      break;
		   default:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      da = _evas_pow_lut[(a << 8) | da];
		      da = ARGB_JOIN(a, da, da, da);
		      movd_m2r(da, mm3);
		      punpcklbw_r2r(mm3, mm3);
		      psrlw_i2r(1, mm3);

		      movd_r2m(mm1, a);
		      a |= 0xff000000;
		      movd_m2r(a, mm1);    // mm1 = [  ][  ][255R][GB] (COL*SRC)
		      punpcklbw_r2r(mm0, mm1);

		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);
	
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
evas_common_blend_pixels_rgba_mul_color_a_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   len = A_VAL(&col);
   col &= 0xff000000;
   col |= RGB_JOIN(len,len,len);
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);
   col |= 0x00ffffff;

   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0: break;
	   case 255:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		      *dst = (*src & col);
		      break;
		   case 255:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      a = *src & col;
		      movd_m2r(a, mm1);
		      movq_r2r(mm1, mm3);
		      punpcklbw_r2r(mm3, mm3);
		      punpckhwd_r2r(mm3, mm3);
		      punpckhdq_r2r(mm3, mm3);
		      psrlw_i2r(1, mm3);
		      psrlq_i2r(16, mm3);

		      punpcklbw_r2r(mm0, mm1);

		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);

		      packuswb_r2r(mm0, mm2);
		      movd_r2m(mm2, *dst);

		      break;
		   default:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      movd_m2r(*src, mm1);
		      punpcklbw_r2r(mm0, mm1);

		      da = _evas_pow_lut[(A_VAL(&col) << 8) + da];
		      da = ARGB_JOIN(A_VAL(&col), da, da, da);
		      movd_m2r(da, mm3);
		      punpcklbw_r2r(mm3, mm3);
		      psrlw_i2r(1, mm3);

		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);

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
//		psrlq_i2r(16, mm3);	

		switch(da)
		  {
		   case 0:
		      *dst = (*src & 0x00ffffff) | (a << 24);
		      break;
		   case 255:
		      movd_m2r(*dst, mm2);
		      punpcklbw_r2r(mm0, mm2);

		      movd_m2r(*src, mm1);
		      punpcklbw_r2r(mm0, mm1);

		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);
	
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

		      a = *src | 0xff000000;
		      movd_m2r(a, mm1);
		      punpcklbw_r2r(mm0, mm1);

		      psubw_r2r(mm2, mm1);
		      psllw_i2r(1, mm1);
		      paddw_r2r(mm6, mm1);
		      pmulhw_r2r(mm3, mm1);
		      paddw_r2r(mm1, mm2);
	
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
evas_common_blend_pixels_rgb_mul_color_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   len = A_VAL(&col);
   col |= 0xff000000;
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm4);
   A_VAL(&col) = len;
   
   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  da = A_VAL(dst);

	switch (da)
	  {
	   case 0:
		movd_m2r(*src, mm2);
		punpcklbw_r2r(mm0, mm2);
		psllw_i2r(1, mm2);
		paddw_r2r(mm6, mm2);
		pmulhw_r2r(mm5, mm2);
		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	   case 255:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		movq_r2r(mm1, mm3);
		punpcklbw_r2r(mm3, mm3);
		punpckhwd_r2r(mm3, mm3);
		punpckhdq_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);

		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);

		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	   default:
	     {
		DATA32  a = *src | 0xff000000;
		
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		movd_m2r(a, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm4, mm1);
		packuswb_r2r(mm0, mm1);

		da = _evas_pow_lut[(A_VAL(&col) << 8) + da];
		da = ARGB_JOIN(A_VAL(&col), da, da, da);
		movd_m2r(da, mm3);
		punpcklbw_r2r(mm3, mm3);   // mm3 = [AA][dAdA][dAdA][dAdA]
		psrlw_i2r(1, mm3);         // mm3 = [AA/2][dAdA/2][dAdA/2][dAdA/2]

		punpcklbw_r2r(mm0, mm1);
		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);

		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);
	     }
		break;
	  }
     }
}

void
evas_common_blend_pixels_rgb_mul_color_a_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   col |= 0x00ffffff;
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);

   movq_r2r(mm3, mm5);

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  da = A_VAL(dst);

	switch (da)
	  {
	   case 0:
		*dst = (*src | 0xff000000) & col;
		break;
	   case 255:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);
		
		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		paddw_r2r(mm1, mm2);

		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	   default:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		da = _evas_pow_lut[(A_VAL(&col) << 8) | da];
		da = ARGB_JOIN(A_VAL(&col), da, da, da);
		movd_m2r(da, mm3);
		punpcklbw_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);   // mm3 = [AA/2][dAdA/2][dAdA/2][dAdA/2]

		da = *src | 0xff000000;
		movd_m2r(da, mm1);
		punpcklbw_r2r(mm0, mm1);

		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);

		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	  }
     }
}

void
evas_common_blend_pixels_rgba_mul_color_rgba_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0: break;
	   case 255:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		movq_r2r(mm1, mm3);
		punpcklbw_r2r(mm3, mm3);
		punpckhwd_r2r(mm3, mm3);
		punpckhdq_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);

		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);
	
		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	   default:

		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		movq_r2r(mm1, mm3);
		punpcklbw_r2r(mm3, mm3);
		punpckhwd_r2r(mm3, mm3);
		punpckhdq_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);

		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);
	
		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	  }
     }
}

void
evas_common_blend_pixels_rgba_mul_color_rgb_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   col |= 0xff000000;
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:  break;
	   case 255:
	        a = (*dst | 0x00ffffff) & *src;
		movd_m2r(a, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);
		movd_r2m(mm1, *dst);

		break;
	   default:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		movq_r2r(mm1, mm3);
		punpcklbw_r2r(mm3, mm3);
		punpckhwd_r2r(mm3, mm3);
		punpckhdq_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);
		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);
	
		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	  }
     }
}

void
evas_common_blend_pixels_rgb_mul_color_rgba_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	movd_m2r(*dst, mm2);
	punpcklbw_r2r(mm0, mm2);

	movd_m2r(*src, mm1);
	punpcklbw_r2r(mm0, mm1);
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm5, mm1);
	packuswb_r2r(mm0, mm1);

	movq_r2r(mm1, mm3);
	punpcklbw_r2r(mm3, mm3);
	punpckhwd_r2r(mm3, mm3);
	punpckhdq_r2r(mm3, mm3);
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	punpcklbw_r2r(mm0, mm1);

	psubw_r2r(mm2, mm1);
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm3, mm1);
	paddw_r2r(mm1, mm2);

	packuswb_r2r(mm0, mm2);
	movd_r2m(mm2, *dst);
     }
}

void
evas_common_blend_pixels_rgba_mul_color_a_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32  *dst_end = (dst + len);

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   len = A_VAL(&col);
   col &= 0xff000000;
   col |= RGB_JOIN(len,len,len);
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);
   col |= 0x00ffffff;

   src--; dst--;
   while(++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0: break;
	   case 255:
		movd_m2r(*dst, mm2);
		punpcklbw_r2r(mm0, mm2);

		a = *src & col;
		movd_m2r(a, mm1);
		movq_r2r(mm1, mm3);
		punpcklbw_r2r(mm3, mm3);
		punpckhwd_r2r(mm3, mm3);
		punpckhdq_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);

		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);
	
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
//		psrlq_i2r(16, mm3);	

		movd_m2r(*src, mm1);
		punpcklbw_r2r(mm0, mm1);

		psubw_r2r(mm2, mm1);
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm3, mm1);
		paddw_r2r(mm1, mm2);
	
		packuswb_r2r(mm0, mm2);
		movd_r2m(mm2, *dst);

		break;
	  }
     }
}

void
evas_common_blend_pixels_rgb_mul_color_a_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   col |= 0x00ffffff;

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = *src & col;

	movd_m2r(*dst, mm2);
	punpcklbw_r2r(mm0, mm2);

	movd_m2r(a, mm1);
	movq_r2r(mm1, mm3);
	punpcklbw_r2r(mm3, mm3);
	punpckhwd_r2r(mm3, mm3);
	punpckhdq_r2r(mm3, mm3);
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	punpcklbw_r2r(mm0, mm1);

	psubw_r2r(mm2, mm1);
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm3, mm1);
	paddw_r2r(mm1, mm2);

	packuswb_r2r(mm0, mm2);
	movd_r2m(mm2, *dst);
     }
}

void
evas_common_copy_pixels_rgb_mul_color_rgb_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   col |= 0xff000000;
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	col = *src | 0xff000000;
	movd_m2r(col, mm1);
	punpcklbw_r2r(mm0, mm1);
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm5, mm1);
	packuswb_r2r(mm0, mm1);
	movd_r2m(mm1, *dst);
     }
}

void
evas_common_copy_pixels_rgb_mul_color_rgb_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   col |= 0xff000000;
   movd_m2r(col, mm3);
   punpcklbw_r2r(mm3, mm3);
   psrlw_i2r(1, mm3);

   movq_r2r(mm3, mm5);

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	col = (*src & 0x00ffffff) | (*dst & 0xff000000);
	movd_m2r(col, mm1);
	punpcklbw_r2r(mm0, mm1);
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm5, mm1);
	packuswb_r2r(mm0, mm1);
	movd_r2m(mm1, *dst);
     }
}
#endif


/****************************************************************************/
void
evas_common_blend_pixels_mul_color_nothing(DATA32 *src, DATA32 *dst, int len, DATA32 col)
{
}
/* ************************************************************************** */


#ifdef BUILD_C
static Gfx_Func_Blend_Src_Mul_Dst
evas_common_gfx_func_blend_col_src_get_c(DATA32 col, RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   if ((col | PIXEL_SOLID_ALPHA) == 0xffffffff)
     {
	if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	    {
//		evas_common_blend_init_evas_pow_lut();
		return evas_common_blend_pixels_rgba_mul_color_a_to_rgba_c;
	    }
	   return evas_common_blend_pixels_rgba_mul_color_a_to_rgb_c;
	  }
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgb_mul_color_a_to_rgba_c;
	  }
	return evas_common_blend_pixels_rgb_mul_color_a_to_rgb_c;
     }

   if (A_VAL(&col) < 255)
     {
	if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	    {
//		evas_common_blend_init_evas_pow_lut();
		return evas_common_blend_pixels_rgba_mul_color_rgba_to_rgba_c;
	    }
	   return evas_common_blend_pixels_rgba_mul_color_rgba_to_rgb_c;
	  }
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgb_mul_color_rgba_to_rgba_c;
	  }
	return evas_common_blend_pixels_rgb_mul_color_rgba_to_rgb_c;
     }

   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgba_mul_color_rgb_to_rgba_c;
	  }
	return evas_common_blend_pixels_rgba_mul_color_rgb_to_rgb_c;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_pixels_rgb_mul_color_rgb_to_rgba_c;
   return evas_common_copy_pixels_rgb_mul_color_rgb_to_rgb_c;
   pixels = 0;
}
#endif

#ifdef BUILD_MMX
static Gfx_Func_Blend_Src_Mul_Dst
evas_common_gfx_func_blend_col_src_get_mmx(DATA32 col, RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   if ((col | PIXEL_SOLID_ALPHA) == 0xffffffff)
     {
	if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	    {
//		evas_common_blend_init_evas_pow_lut();
		return evas_common_blend_pixels_rgba_mul_color_a_to_rgba_mmx;
	    }
	   return evas_common_blend_pixels_rgba_mul_color_a_to_rgb_mmx;
	  }
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgb_mul_color_a_to_rgba_mmx;
	  }
	return evas_common_blend_pixels_rgb_mul_color_a_to_rgb_mmx;
     }

   if (A_VAL(&col) < 255)
     {
	if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	    {
//		evas_common_blend_init_evas_pow_lut();
		return evas_common_blend_pixels_rgba_mul_color_rgba_to_rgba_mmx;
	    }
	   return evas_common_blend_pixels_rgba_mul_color_rgba_to_rgb_mmx;
	  }
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgb_mul_color_rgba_to_rgba_mmx;
	  }
	return evas_common_blend_pixels_rgb_mul_color_rgba_to_rgb_mmx;
     }

   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgba_mul_color_rgb_to_rgba_mmx;
	  }
	return evas_common_blend_pixels_rgba_mul_color_rgb_to_rgb_mmx;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_pixels_rgb_mul_color_rgb_to_rgba_mmx;
   return evas_common_copy_pixels_rgb_mul_color_rgb_to_rgb_mmx;
   pixels = 0;
}
#endif


Gfx_Func_Blend_Src_Mul_Dst
evas_common_draw_func_blend_mul_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels)
{
   Gfx_Func_Blend_Src_Mul_Dst  func = NULL;

#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	func = evas_common_gfx_func_blend_col_src_get_mmx(col, src, dst, pixels);
	if (func)  return func;
     }
#endif
#ifdef BUILD_C
   func = evas_common_gfx_func_blend_col_src_get_c(col, src, dst, pixels);
   if (func)  return func;
#endif
   return evas_common_blend_pixels_mul_color_nothing;
}

