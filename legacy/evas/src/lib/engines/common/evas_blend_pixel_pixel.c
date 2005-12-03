#include "evas_common.h"

#if defined BUILD_MMX || defined BUILD_SSE
#include "evas_mmx.h"
#endif

#define ALIGN_FIX

// extern DATA8       *_evas_pow_lut;
extern const DATA8  _evas_pow_lut[65536];
extern const DATA16 _evas_const_c1[4];

#ifdef BUILD_C
void
evas_common_blend_pixels_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst = *src;
	     break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		     *dst = *src;
		     break;
		   case 255:
		     a += a >> 7;
		     *dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * a) >> 8,
		                       ((G_VAL(src) - G_VAL(dst)) * a) >> 8,
		                       ((B_VAL(src) - B_VAL(dst)) * a) >> 8 );
		     break;
		   default:
		     da = _evas_pow_lut[(a << 8) + da];
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
evas_common_blend_pixels_rgba_to_rgb_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst = (*dst | 0x00ffffff) & (*src);
	     break;
	   default:
	     a += a >> 7;
	     *dst += RGB_JOIN( ((R_VAL(src) - R_VAL(dst)) * a) >> 8,
                               ((G_VAL(src) - G_VAL(dst)) * a) >> 8,
                               ((B_VAL(src) - B_VAL(dst)) * a) >> 8 );
	     break;
	  }
     }
}

void
evas_common_copy_pixels_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   while (dst < dst_end)
	*dst++ = *src++;
}

void
evas_common_copy_pixels_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   while (dst < dst_end)
	*dst++ = *src++ | PIXEL_SOLID_ALPHA;
}

void
evas_common_copy_pixels_rgb_to_rgb_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   while (dst < dst_end)
	*dst++ = (*src++ | PIXEL_SOLID_ALPHA) & (*dst | 0x00ffffff);
}

void
evas_common_copy_pixels_rev_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_start = dst;

   src = src + len - 1;
   dst = dst + len - 1;

   while (dst >= dst_start)
	*dst-- = *src--;
}


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
#endif

/****************************************************************************/

#ifdef BUILD_MMX
void
evas_common_blend_pixels_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst = *src;
	     break;
	   default:
	     {
		DATA32  da = A_VAL(dst);

		switch(da)
		  {
		   case 0:
		     *dst = *src;
		     break;
		   case 255:
		     movd_m2r(*src, mm1);       // mm1 = [  ][  ][AR][GB] (SRC)
		     movd_m2r(*dst, mm2);       // mm2 = [  ][  ][ar][gb] (DST)

		     movq_r2r(mm1, mm3);
		     punpcklbw_r2r(mm3, mm3);
		     punpckhwd_r2r(mm3, mm3);
		     punpckhdq_r2r(mm3, mm3);
		     psrlw_i2r(1, mm3);
//		     psrlq_i2r(16, mm3);

		     punpcklbw_r2r(mm0, mm1);
		     punpcklbw_r2r(mm0, mm2);

		     psubw_r2r(mm2, mm1);
		     psllw_i2r(1, mm1);
		     paddw_r2r(mm6, mm1);
		     pmulhw_r2r(mm3, mm1);
		     paddw_r2r(mm1, mm2);

		     packuswb_r2r(mm0, mm2);
		     movd_r2m(mm2, *dst);
		   
		     break;
		   default:
		     da = _evas_pow_lut[(a << 8) + da];
		     da = ARGB_JOIN(a, da, da, da);
		     movd_m2r(da, mm3);
		     punpcklbw_r2r(mm3, mm3);   // mm3 = [AA][dAdA][dAdA][dAdA]
		     psrlw_i2r(1, mm3);         // mm3 = [AA/2][dAdA/2][dAdA/2][dAdA/2]

		     a = *src | 0xff000000;
		     movd_m2r(a, mm1);          // mm1 = [  ][  ][255R][GB] (SRC)
		     punpcklbw_r2r(mm0, mm1);
		     movd_m2r(*dst, mm2);       // mm1 = [  ][  ][ar][gb] (DST)
		     punpcklbw_r2r(mm0, mm2);

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
evas_common_blend_pixels_rgba_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   pxor_r2r(mm0, mm0);
   movq_m2r(*_evas_const_c1, mm6);

   src--; dst--;
   while (++src, ++dst < dst_end)
     {
	DATA32  a = A_VAL(src);

	switch (a)
	  {
	   case 0:
	     break;
	   case 255:
	     *dst = (*dst | 0x00ffffff) & (*src);
	     break;
	   default:
	     movd_m2r(*src, mm1);       // mm1 = [  ][  ][AR][GB] (SRC)
	     movd_m2r(*dst, mm2);       // mm2 = [  ][  ][ar][gb] (DST)

	     movq_r2r(mm1, mm3);        // mm3 = [  ][  ][AR][GB]
	     punpcklbw_r2r(mm3, mm3);   // mm3 = [AA][RR][GG][BB]
	     punpckhwd_r2r(mm3, mm3);   // mm3 = [AA][AA][RR][RR]
	     punpckhdq_r2r(mm3, mm3);	// mm3 = [AA][AA][AA][AA]
	     psrlw_i2r(1, mm3);         // mm3 = [AA/2][AA/2][AA/2][AA/2]
//	     psrlq_i2r(16, mm3);        // mm3 = [00][AA/2][AA/2][AA/2]

	     punpcklbw_r2r(mm0, mm1);   // mm1 = [0A][0R][0G][0B]
	     punpcklbw_r2r(mm0, mm2);   // mm2 = [0a][0r][0g][0b]

	     psubw_r2r(mm2, mm1);       // mm1 = [A-a][R-r][G-g][B-b]
	     psllw_i2r(1, mm1);         // mm1 = [A*2][R*2][G*2][B*2]
	     paddw_r2r(mm6, mm1);       // mm1 = [A+1][R+1][G+1][B+1]
	     pmulhw_r2r(mm3, mm1);      // mm1 = [A*0][(R*AA)>>16][(G*AA)>>16][(B*AA)>>16]
	     paddw_r2r(mm1, mm2);       // mm2 = [0a][R-r][G-g][B-b]

	     packuswb_r2r(mm0, mm2);    // mm2 = [  ][  ][AR][GB]
	     movd_r2m(mm2, *dst);       // DST = mm2
	   
	     break;
	  }
     }
}

void
evas_common_copy_pixels_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
#ifdef ALIGN_FIX
   int src_align;
   int dst_align;

   src_align = (int)src & 0x3f; /* 64 byte alignment */
   dst_align = (int)dst & 0x3f; /* 64 byte alignment */

   if ((src_align != dst_align) ||
       ((src_align & 0x3) != 0))
     {
#ifdef BUILD_C
	evas_common_copy_pixels_rgba_to_rgba_c(src, dst, len);
	return;
     }
#endif

   while ((src_align > 0) && (len > 0))
     {
	*dst = *src;
	dst++;
	src++;
	len--;
	src_align -= sizeof(DATA32);
     }
#endif /* ALIGN_FIX */

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

void
evas_common_copy_pixels_rgba_to_rgba_mmx2(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
#ifdef ALIGN_FIX
   int src_align;
   int dst_align;

   src_align = (int)src & 0x3f; /* 64 byte alignment */
   dst_align = (int)dst & 0x3f; /* 64 byte alignment */

   if ((src_align != dst_align) ||
       ((src_align & 0x3) != 0))
     {
#ifdef BUILD_C
	evas_common_copy_pixels_rgba_to_rgba_c(src, dst, len);
#endif
	return;
     }

   while ((src_align > 0) && (len > 0))
     {
	*dst = *src;
	dst++;
	src++;
	len--;
	src_align -= sizeof(DATA32);
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

#define evas_common_copy_pixels_rgb_to_rgb_mmx  evas_common_copy_pixels_rgba_to_rgba_mmx
#define evas_common_copy_pixels_rgb_to_rgba_mmx  NULL

#define evas_common_copy_pixels_rgb_to_rgb_mmx2  evas_common_copy_pixels_rgba_to_rgba_mmx2
#define evas_common_copy_pixels_rgb_to_rgba_mmx2  NULL
#endif


/****************************************************************************/


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

   if ((src_align != dst_align) || 
       ((src_align & 0x3) != 0))
     {
#ifdef BUILD_C
	evas_common_copy_pixels_rgba_to_rgba_c(src, dst, len);
#endif
	return;
     }

   while ((src_align > 0) && (len > 0))
     {
	*dst = *src;
	dst++;
	src++;
	len--;
	src_align -= sizeof(DATA32);
     }
#endif /* ALIGN_FIX */
   
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

#define evas_common_blend_pixels_rgba_to_rgba_sse  NULL
#define evas_common_blend_pixels_rgba_to_rgb_sse  NULL
#define evas_common_copy_pixels_rgb_to_rgb_sse  evas_common_copy_pixels_rgba_to_rgba_sse
#define evas_common_copy_pixels_rgb_to_rgba_sse  NULL
#endif


/****************************************************************************/
static void
evas_common_blend_pixels_nothing(DATA32 *src, DATA32 *dst, int len)
{
}

static void
evas_common_copy_pixels_nothing(DATA32 *src, DATA32 *dst, int len)
{
}
/****************************************************************************/

#ifdef BUILD_SSE
static Gfx_Func_Blend_Src_Dst
evas_common_gfx_func_blend_pixels_get_sse(RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	   return evas_common_blend_pixels_rgba_to_rgba_sse;
	return evas_common_blend_pixels_rgba_to_rgb_sse;
     }
   if (pixels <= (256 * 256))
	return NULL;
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_pixels_rgb_to_rgba_sse;
   return evas_common_copy_pixels_rgb_to_rgb_sse;
}
#endif

#ifdef BUILD_MMX
static Gfx_Func_Blend_Src_Dst
evas_common_gfx_func_blend_pixels_get_mmx(RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgba_to_rgba_mmx;
	  }
	return evas_common_blend_pixels_rgba_to_rgb_mmx;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_pixels_rgb_to_rgba_mmx;
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
	return evas_common_copy_pixels_rgb_to_rgb_mmx2;
   return evas_common_copy_pixels_rgb_to_rgb_mmx;
   pixels = 0;
}
#endif

#ifdef BUILD_C
static Gfx_Func_Blend_Src_Dst
evas_common_gfx_func_blend_pixels_get_c(RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
//	    evas_common_blend_init_evas_pow_lut();
	    return evas_common_blend_pixels_rgba_to_rgba_c;
	  }
	return evas_common_blend_pixels_rgba_to_rgb_c;
     }
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	return evas_common_copy_pixels_rgb_to_rgba_c;
   return evas_common_copy_pixels_rgb_to_rgb_c;
   pixels = 0;
}
#endif

Gfx_Func_Blend_Src_Dst
evas_common_draw_func_blend_get (RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   Gfx_Func_Blend_Src_Dst  func = NULL;

#ifdef BUILD_SSE
   if (evas_common_cpu_has_feature(CPU_FEATURE_SSE))
     {
	func = evas_common_gfx_func_blend_pixels_get_sse(src, dst, pixels);
	if (func)  return func;
     }
#endif
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	func = evas_common_gfx_func_blend_pixels_get_mmx(src, dst, pixels);
	if (func)  return func;
     }
#endif
#ifdef BUILD_C
   func = evas_common_gfx_func_blend_pixels_get_c(src, dst, pixels);
   if (func) return func;
#endif
   return evas_common_blend_pixels_nothing;
}

/* ********************************************* */

#ifdef BUILD_SSE
static Gfx_Func_Blend_Src_Dst
evas_common_gfx_func_copy_get_sse(int pixels, int reverse)
{
   if (pixels <= (256 * 256))
	  return NULL;
   if (reverse)
	  return evas_common_copy_pixels_rev_rgba_to_rgba_sse;
   return evas_common_copy_pixels_rgba_to_rgba_sse;
}
#endif

#ifdef BUILD_MMX
static Gfx_Func_Blend_Src_Dst
evas_common_gfx_func_copy_get_mmx(int pixels, int reverse)
{
   if (reverse)
	 return evas_common_copy_pixels_rev_rgba_to_rgba_mmx;
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
	 return evas_common_copy_pixels_rgba_to_rgba_mmx2;
   return evas_common_copy_pixels_rgba_to_rgba_mmx;
   pixels = 0;
}
#endif

#ifdef BUILD_C
static Gfx_Func_Blend_Src_Dst
evas_common_gfx_func_copy_get_c(int pixels, int reverse)
{
   if (reverse)
	  return evas_common_copy_pixels_rev_rgba_to_rgba_c;
   return evas_common_copy_pixels_rgba_to_rgba_c;
   pixels = 0;
}
#endif


Gfx_Func_Blend_Src_Dst
evas_common_draw_func_copy_get(int pixels, int reverse)
{
   Gfx_Func_Blend_Src_Dst  func = NULL;

#ifdef BUILD_SSE
   if (evas_common_cpu_has_feature(CPU_FEATURE_SSE))
     {
	func = evas_common_gfx_func_copy_get_sse(pixels, reverse);
	if (func)  return func;
     }
#endif
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	func = evas_common_gfx_func_copy_get_mmx(pixels, reverse);
	if (func)  return func;
     }
#endif
#ifdef BUILD_C
   func = evas_common_gfx_func_copy_get_c(pixels, reverse);
   if (func)  return func;
#endif
   return evas_common_copy_pixels_nothing;
}

