#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

#define ALIGN_FIX

extern DATA8       *_evas_pow_lut;
extern const DATA16 _evas_const_c1[4];

#ifdef BUILD_C
void
evas_common_blend_pixels_rgba_to_rgb_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8  a;
	
	a = A_VAL(src_ptr);

	switch (a)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst_ptr = *src_ptr;
	     break;
	   default:
	     BLEND_ALPHA_SETUP(a, tmp);
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
#endif

#ifdef BUILD_MMX
void
evas_common_blend_pixels_rgba_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   
   pxor_r2r(mm4, mm4);
   movq_m2r(*_evas_const_c1, mm5);
   
   while (dst_ptr < dst_end_ptr)
     {
	switch (A_VAL(src_ptr))
	  {
	   case 0:
	     break;
	   case 255:
	     *dst_ptr = *src_ptr;
	     break;
	   default:
	     movd_m2r(src_ptr[0], mm1); // mm1 = [  ][  ][AR][GB] (SRC)
	     movd_m2r(dst_ptr[0], mm2); // mm2 = [  ][  ][ar][gb] (DST)
	     
	     movq_r2r(mm1, mm3);        // mm3 = [  ][  ][AR][GB]
	     punpcklbw_r2r(mm3, mm3);   // mm3 = [AA][RR][GG][BB]
	     punpckhwd_r2r(mm3, mm3);   // mm3 = [AA][AA][RR][RR]
	     punpckhdq_r2r(mm3, mm3);	// mm3 = [AA][AA][AA][AA]
	     psrlw_i2r(1, mm3);         // mm3 = [AA/2][AA/2][AA/2][AA/2]
	     
//	     psrlq_i2r(16, mm3);        // mm3 = [00][AA/2][AA/2][AA/2]
	     
	     punpcklbw_r2r(mm4, mm1);   // mm1 = [0A][0R][0G][0B]
	     punpcklbw_r2r(mm4, mm2);   // mm2 = [0a][0r][0g][0b]
	     
	     psubw_r2r(mm2, mm1);       // mm1 = [A-a][R-r][G-g][B-b]
	     psllw_i2r(1, mm1);         // mm1 = [A*2][R*2][G*2][B*2]
	     paddw_r2r(mm5, mm1);       // mm1 = [A+1][R+1][G+1][B+1]
	     pmulhw_r2r(mm3, mm1);      // mm1 = [A*0][(R*AA)>>16][(G*AA)>>16][(B*AA)>>16]
	     paddw_r2r(mm1, mm2);       // mm2 = [0a][R-r][G-g][B-b]

	     packuswb_r2r(mm4, mm2);    // mm2 = [  ][  ][AR][GB]
	     movd_r2m(mm2, dst_ptr[0]); // DST = mm2
	     break;
	  }
	src_ptr++;
	dst_ptr++;
     }
}
#endif

void
evas_common_blend_pixels_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8  a, aa;
	
	aa = A_VAL(src_ptr);
	switch (aa)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst_ptr = *src_ptr;
	     break;
	   default:
	     BLEND_ADST_ALPHA_SETUP(aa, tmp);
	     a = _evas_pow_lut[(aa << 8) | A_VAL(dst_ptr)];
	     BLEND_ADST_COLOR(aa, A_VAL(dst_ptr), 
			      255, A_VAL(dst_ptr), 
			      tmp);
	     BLEND_ADST_ALPHA_SETUP(a, tmp);
	     BLEND_ADST_COLOR(a, R_VAL(dst_ptr), 
			      R_VAL(src_ptr), R_VAL(dst_ptr), 
			      tmp);
	     BLEND_ADST_COLOR(a, G_VAL(dst_ptr), 
			      G_VAL(src_ptr), G_VAL(dst_ptr), 
			      tmp);
	     BLEND_ADST_COLOR(a, B_VAL(dst_ptr), 
			      B_VAL(src_ptr), B_VAL(dst_ptr), 
			      tmp);
	  }
	src_ptr++;
	dst_ptr++;
     }
}

/****************************************************************************/

#ifdef BUILD_C
void
evas_common_copy_pixels_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = *src_ptr;
	src_ptr++;
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_MMX
void
evas_common_copy_pixels_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
#ifdef ALIGN_FIX   
   int src_align;
   int dst_align;

   src_align = (int)src & 0x3f; /* 64 byte alignment */
   dst_align = (int)dst & 0x3f; /* 64 byte alignment */
   if ((src_align != 0) && 
       (!(src_align & 0x3)) && 
       (src_align == dst_align))
     {
	while ((src_align > 0) && (len > 0))
	  {
	     *dst = *src;
	     dst++;
	     src++;
	     len--;
	     src_align -= sizeof(DATA32);
	  }
     }
   else
     {
#ifdef BUILD_C
	evas_common_copy_pixels_rgba_to_rgba_c(src, dst, len);
#endif
	return;
     }
#endif   
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   dst_end_ptr_pre = dst + ((len / 16) * 16);
   
   while (dst_ptr < dst_end_ptr_pre)
     {
	MOVE_16DWORDS_MMX(src_ptr, dst_ptr);
	src_ptr+=16;
	dst_ptr+=16;
     }
   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = *src_ptr;
	src_ptr++;
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_MMX
void
evas_common_copy_pixels_rgba_to_rgba_mmx2(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
#ifdef ALIGN_FIX   
   int src_align;
   int dst_align;

   src_align = (int)src & 0x3f; /* 64 byte alignment */
   dst_align = (int)dst & 0x3f; /* 64 byte alignment */
   if ((src_align != 0) && 
       (!(src_align & 0x3)) && 
       (src_align == dst_align))
     {
	while ((src_align > 0) && (len > 0))
	  {
	     *dst = *src;
	     dst++;
	     src++;
	     len--;
	     src_align -= sizeof(DATA32);
	  }
     }
   else
     {
#ifdef BUILD_C
	evas_common_copy_pixels_rgba_to_rgba_c(src, dst, len);
#endif
	return;
     }
#endif
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   dst_end_ptr_pre = dst + ((len / 16) * 16);
   
   while (dst_ptr < dst_end_ptr_pre)
     {
	MOVE_16DWORDS_MMX(src_ptr, dst_ptr);
	src_ptr+=16;
	dst_ptr+=16;
     }
   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = *src_ptr;
	src_ptr++;
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_SSE
void
evas_common_copy_pixels_rgba_to_rgba_sse(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
#ifdef ALIGN_FIX   
   int src_align;
   int dst_align;

   src_align = (int)src & 0x3f; /* 64 byte alignment */
   dst_align = (int)dst & 0x3f; /* 64 byte alignment */
   if ((src_align != 0) && 
       (!(src_align & 0x3)) && 
       (src_align == dst_align))
     {
	while ((src_align > 0) && (len > 0))
	  {
	     *dst = *src;
	     dst++;
	     src++;
	     len--;
	     src_align -= sizeof(DATA32);
	  }
     }
   else
     {
#ifdef BUILD_C
	evas_common_copy_pixels_rgba_to_rgba_c(src, dst, len);
#endif
	return;
     }
#endif
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   dst_end_ptr_pre = dst + ((len / 16) * 16);

   while (dst_ptr < dst_end_ptr_pre)
     {
	prefetch(&src_ptr[16]);
	MOVE_16DWORDS_MMX(src_ptr, dst_ptr);
	src_ptr+=16;
	dst_ptr+=16;
     }
   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = *src_ptr;
	src_ptr++;
	dst_ptr++;
     }
}
#endif

void
evas_common_copy_pixels_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = *src_ptr | PIXEL_SOLID_ALPHA;
	src_ptr++;
	dst_ptr++;
     }
}

/****************************************************************************/

#ifdef BUILD_C
void
evas_common_copy_pixels_rev_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src + len - 1;
   dst_ptr = dst + len - 1;
   dst_end_ptr = dst;

   while (dst_ptr >= dst_end_ptr)
     {
	*dst_ptr = *src_ptr;
	src_ptr--;
	dst_ptr--;
     }
}
#endif

#ifdef BUILD_MMX
void
evas_common_copy_pixels_rev_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
   src_ptr = src + len - 16;
   dst_ptr = dst + len - 16;
   dst_end_ptr = dst;
   dst_end_ptr_pre = dst + len - ((len / 16) * 16);
   
   if (len >= 16)
     {
	while (dst_ptr >= dst_end_ptr_pre)
	  {
	     MOVE_16DWORDS_MMX(src_ptr, dst_ptr);
	     src_ptr-=16;
	     dst_ptr-=16;
	  }
	src_ptr+=15;
	dst_ptr+=15;
	while (dst_ptr >= dst_end_ptr)
	  {
	     *dst_ptr = *src_ptr;
	     src_ptr--;
	     dst_ptr--;
	  }
     }
   else
     {
	src_ptr = src + len - 1;
	dst_ptr = dst + len - 1;	
	while (dst_ptr >= dst_end_ptr)
	  {
	     *dst_ptr = *src_ptr;
	     src_ptr--;
	     dst_ptr--;
	  }
     }
}
#endif   

#ifdef BUILD_SSE
void
evas_common_copy_pixels_rev_rgba_to_rgba_sse(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
   src_ptr = src + len - 16;
   dst_ptr = dst + len - 16;
   dst_end_ptr = dst;
   dst_end_ptr_pre = dst + len - ((len / 16) * 16);

   if (len >= 16)
     {
	while (dst_ptr >= dst_end_ptr_pre)
	  {
	     prefetch(&src_ptr[-16]);
	     MOVE_10DWORDS_MMX(src_ptr, dst_ptr);
	     src_ptr-=16;
	     dst_ptr-=16;
	  }
	src_ptr+=15;
	dst_ptr+=15;
	while (dst_ptr >= dst_end_ptr)
	  {
	     *dst_ptr = *src_ptr;
	     src_ptr--;
	     dst_ptr--;
	  }
     }
   else
     {
	src_ptr = src + len - 1;
	dst_ptr = dst + len - 1;	
	while (dst_ptr >= dst_end_ptr)
	  {
	     *dst_ptr = *src_ptr;
	     src_ptr--;
	     dst_ptr--;
	  }
     }
}
#endif

void
evas_common_copy_pixels_rev_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src + len - 1;
   dst_ptr = dst + len - 1;
   dst_end_ptr = dst;

   while (dst_ptr >= dst_end_ptr)
     {
	*dst_ptr = *src_ptr | PIXEL_SOLID_ALPHA;
	src_ptr--;
	dst_ptr--;
     }
}
