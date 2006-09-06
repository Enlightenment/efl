#include "evas_common.h"

#if defined BUILD_MMX || defined BUILD_SSE
#include "evas_mmx.h"
#endif

#define ALIGN_FIX

static void evas_common_copy_pixels_c        (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_mmx      (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_mmx2     (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_sse/*NB*/ (DATA32 *src, DATA32 *dst, int len);

static void evas_common_copy_pixels_rev_c           (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_rev_mmx         (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_rev_sse/*NB*/ (DATA32 *src, DATA32 *dst, int len);


EAPI void
evas_common_blit_init(void)
{
}

EAPI void
evas_common_blit_rectangle(RGBA_Image *src, RGBA_Image *dst, int src_x, int src_y, int w, int h, int dst_x, int dst_y)
{
   int y;
   Gfx_Func_Copy func;
   DATA32 *src_ptr, *dst_ptr;

   /* clip clip clip */
   if (w <= 0) return;
   if (src_x + w > src->image->w) w = src->image->w - src_x;
   if (w <= 0) return;
   if (src_x < 0)
     {
	dst_x -= src_x;
	w += src_x;
	src_x = 0;
     }
   if (w <= 0) return;

   if (h <= 0) return;
   if (src_y + h > src->image->h) h = src->image->h - src_y;
   if (h <= 0) return;
   if (src_y < 0)
     {
	dst_y -= src_y;
	h += src_y;
	src_y = 0;
     }
   if (h <= 0) return;

   if (w <= 0) return;
   if (dst_x + w > dst->image->w) w = dst->image->w - dst_x;
   if (w <= 0) return;
   if (dst_x < 0)
     {
	src_x -= dst_x;
	w += dst_x;
	dst_x = 0;
     }
   if (w <= 0) return;

   if (h <= 0) return;
   if (dst_y + h > dst->image->h) h = dst->image->h - dst_y;
   if (h <= 0) return;
   if (dst_y < 0)
     {
	src_y -= dst_y;
	h += dst_y;
	dst_y = 0;
     }
   if (h <= 0) return;

   if (dst == src)
     {
	/* src after dst - go forward */
	if (((src_y * src->image->w) + src_x) > ((dst_y * dst->image->w) + dst_x))
	  {
	     func = evas_common_draw_func_copy_get(w, 0);
	     for (y = 0; y < h; y++)
	       {
		  src_ptr = src->image->data + ((y + src_y) * src->image->w) + src_x;
		  dst_ptr = dst->image->data + ((y + dst_y) * dst->image->w) + dst_x;
		  func(src_ptr, dst_ptr, w);
	       }
	  }
	/* reverse */
	else
	  {
	     func = evas_common_draw_func_copy_get(w, 1);
	     for (y = h - 1; y >= 0; y--)
	       {
		  src_ptr = src->image->data + ((y + src_y) * src->image->w) + src_x;
		  dst_ptr = dst->image->data + ((y + dst_y) * dst->image->w) + dst_x;
		  func(src_ptr, dst_ptr, w);
	       }
	  }
     }
   else
     {
	func = evas_common_draw_func_copy_get(w, 0);
	for (y = 0; y < h; y++)
	  {
	     src_ptr = src->image->data + ((y + src_y) * src->image->w) + src_x;
	     dst_ptr = dst->image->data + ((y + dst_y) * dst->image->w) + dst_x;
	     func(src_ptr, dst_ptr, w);
	  }
     }
}


/****************************************************************************/

#ifdef BUILD_C
static void
evas_common_copy_pixels_c(DATA32 *src, DATA32 *dst, int len)
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
static void
evas_common_copy_pixels_mmx(DATA32 *src, DATA32 *dst, int len)
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
	evas_common_copy_pixels_c(src, dst, len);
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
#endif

#ifdef BUILD_MMX
static void
evas_common_copy_pixels_mmx2(DATA32 *src, DATA32 *dst, int len)
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
	evas_common_copy_pixels_c(src, dst, len);
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
#endif

#ifdef BUILD_SSE
static void
evas_common_copy_pixels_sse(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;

   dst_end_ptr = dst + len;
   dst_end_ptr -= 15;
   src_ptr = src;
   dst_ptr = dst;
   while (dst_ptr < dst_end_ptr)
     {
	MOVE_16DWORDS_MMX2(src_ptr, dst_ptr);
	src_ptr+=16;
	dst_ptr+=16;
     }
   dst_end_ptr = dst + len;
   while (dst_ptr < dst_end_ptr)
     {
	*dst_ptr = *src_ptr;
	src_ptr++;
	dst_ptr++;
     }
#if 0   
#ifdef ALIGN_FIX
   int src_align;
   int dst_align;

   src_align = (int)src & 0x3f; /* 64 byte alignment */
   dst_align = (int)dst & 0x3f; /* 64 byte alignment */

   if ((src_align != dst_align) ||
       ((src_align & 0x3) != 0))
     {
#ifdef BUILD_C
	evas_common_copy_pixels_c(src, dst, len);
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
#endif   
}
#endif

/****************************************************************************/

#ifdef BUILD_C
static void
evas_common_copy_pixels_rev_c(DATA32 *src, DATA32 *dst, int len)
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
static void
evas_common_copy_pixels_rev_mmx(DATA32 *src, DATA32 *dst, int len)
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
static void
evas_common_copy_pixels_rev_sse(DATA32 *src, DATA32 *dst, int len)
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


Gfx_Func_Copy
evas_common_draw_func_copy_get(int pixels, int reverse)
{
   if (reverse)
     {
#ifdef  BUILD_SSE
	if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 256 * 256))
	  return evas_common_copy_pixels_rev_sse;
#endif
#ifdef BUILD_MMX
# ifdef BUILD_SSE
	else
# endif
	  if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
	    return evas_common_copy_pixels_rev_mmx;
#endif
#ifdef BUILD_C
# ifdef BUILD_MMX
	else
# endif
	  return evas_common_copy_pixels_rev_c;
#endif
     }
   else
     {
#if 1

# ifdef BUILD_MMX
# ifdef BUILD_C
	     if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
# endif
	       return evas_common_copy_pixels_mmx2;
# ifdef BUILD_SSE
	     else
# endif
#endif
#ifdef BUILD_SSE
# ifdef BUILD_C
	       if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 64 * 64))
# endif
		 return evas_common_copy_pixels_sse;
# ifdef BUILD_MMX
	     else
# endif
#endif
#ifdef BUILD_MMX
# ifdef BUILD_C
	       if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
# endif
		 return evas_common_copy_pixels_mmx;
# ifdef BUILD_C
	     else
# endif
#endif
#ifdef BUILD_C
	       return evas_common_copy_pixels_c;
#endif

#else

# ifdef BUILD_SSE
	     if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 256 * 256))
	       return evas_common_copy_pixels_sse;
# ifdef BUILD_MMX
	     else
# endif
#endif
#ifdef BUILD_MMX
# ifdef BUILD_C
	       if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
# endif
		 return evas_common_copy_pixels_mmx2;
# ifdef BUILD_C
	       else if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
# endif
		 return evas_common_copy_pixels_mmx;
# ifdef BUILD_C
	     else
# endif
#endif
#ifdef BUILD_C
	       return evas_common_copy_pixels_c;
#endif

#endif
     }
#ifdef BUILD_C
   return evas_common_copy_pixels_c;
#else
   return NULL;
#endif
   pixels = 0;
}
