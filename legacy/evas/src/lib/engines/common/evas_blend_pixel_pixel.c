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
	     movd_m2r(src_ptr[0], mm1);
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
	     a = _evas_pow_lut[(aa << 8) | A_VAL(dst_ptr)];
	     
	     BLEND_COLOR(aa, A_VAL(dst_ptr), 
			 255, A_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, R_VAL(dst_ptr), 
			 R_VAL(src_ptr), R_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, G_VAL(dst_ptr), 
			 G_VAL(src_ptr), G_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, B_VAL(dst_ptr), 
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
   dst_end_ptr_pre = dst + ((len / 10) * 10);
   
   while (dst_ptr < dst_end_ptr_pre)
     {
	MOVE_10DWORDS_MMX(src_ptr, dst_ptr);
	src_ptr+=10;
	dst_ptr+=10;
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
	MOVE_16DWORDS_MMX2(src_ptr, dst_ptr);
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
   dst_end_ptr_pre = dst + ((len / 10) * 10);

   while (dst_ptr < dst_end_ptr_pre)
     {
	prefetch(&src_ptr[128]);
	prefetch(&dst_ptr[128]);
	MOVE_10DWORDS_MMX(src_ptr, dst_ptr);
	src_ptr+=10;
	dst_ptr+=10;
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
   
   src_ptr = src + len - 10;
   dst_ptr = dst + len - 10;
   dst_end_ptr = dst;
   dst_end_ptr_pre = dst + len - ((len / 10) * 10);
   
   if (len >= 10)
     {
	while (dst_ptr >= dst_end_ptr_pre)
	  {
	     MOVE_10DWORDS_MMX(src_ptr, dst_ptr);
	     src_ptr-=10;
	     dst_ptr-=10;
	  }
	src_ptr+=9;
	dst_ptr+=9;
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
   
   src_ptr = src + len - 10;
   dst_ptr = dst + len - 10;
   dst_end_ptr = dst;
   dst_end_ptr_pre = dst + len - ((len / 10) * 10);

   if (len >= 10)
     {
	while (dst_ptr >= dst_end_ptr_pre)
	  {
	     prefetch(&src_ptr[-128]);
	     prefetch(&dst_ptr[-128]);
	     MOVE_10DWORDS_MMX(src_ptr, dst_ptr);
	     src_ptr-=10;
	     dst_ptr-=10;
	  }
	src_ptr+=9;
	dst_ptr+=9;
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
