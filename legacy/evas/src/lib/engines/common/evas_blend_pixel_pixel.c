#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

extern DATA8        pow_lut[256][256];
extern const DATA16 const_c1[4];

#ifdef BUILD_C
void
blend_pixels_rgba_to_rgb_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;

	if (A_VAL(src_ptr)) /* hmmm - do we need this? */
	  {
	     BLEND_COLOR(A_VAL(src_ptr), R_VAL(dst_ptr), 
			 R_VAL(src_ptr), R_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(A_VAL(src_ptr), G_VAL(dst_ptr), 
			 G_VAL(src_ptr), G_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(A_VAL(src_ptr), B_VAL(dst_ptr), 
			 B_VAL(src_ptr), B_VAL(dst_ptr), 
			 tmp);
	  }
	src_ptr++;
	dst_ptr++;
     }
}
#endif

#ifdef BUILD_MMX
void
blend_pixels_rgba_to_rgb_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;
   
   pxor_r2r(mm4, mm4);
   movq_m2r(*const_c1, mm5);
   
   while (dst_ptr < dst_end_ptr)
     {
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
	
	src_ptr++;
	dst_ptr++;
     }
}
#endif

void
blend_pixels_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8  a;
	
	if (A_VAL(src_ptr)) /* hmmm - do we need this? */
	  {
	     a = pow_lut[A_VAL(src_ptr)][A_VAL(dst_ptr)];
	     
	     BLEND_COLOR(a, R_VAL(dst_ptr), 
			 R_VAL(src_ptr), R_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, G_VAL(dst_ptr), 
			 G_VAL(src_ptr), G_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, B_VAL(dst_ptr), 
			 B_VAL(src_ptr), B_VAL(dst_ptr), 
			 tmp);	
	     A_VAL(dst_ptr) = A_VAL(dst_ptr) + ((A_VAL(src_ptr) * (255 - A_VAL(dst_ptr))) / 255);
	  }
	src_ptr++;
	dst_ptr++;
     }
}

/****************************************************************************/

#ifdef BUILD_C
void
copy_pixels_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
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
copy_pixels_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
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

#ifdef BUILD_SSE
void
copy_pixels_rgba_to_rgba_sse(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr, *dst_end_ptr_pre;
   
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
copy_pixels_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
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
copy_pixels_rev_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
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
copy_pixels_rev_rgba_to_rgba_mmx(DATA32 *src, DATA32 *dst, int len)
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
copy_pixels_rev_rgba_to_rgba_sse(DATA32 *src, DATA32 *dst, int len)
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
copy_pixels_rev_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len)
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
