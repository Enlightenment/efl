#include "evas_common_private.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

#define ALIGN_FIX

static void evas_common_copy_pixels_c        (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_mmx      (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_mmx2     (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_sse/*NB*/ (DATA32 *src, DATA32 *dst, int len);

#ifdef BUILD_NEON
static void evas_common_copy_pixels_neon     (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_rev_neon (DATA32 *src, DATA32 *dst, int len);
#endif

static void evas_common_copy_pixels_rev_c           (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_rev_mmx         (DATA32 *src, DATA32 *dst, int len);
static void evas_common_copy_pixels_rev_sse/*NB*/ (DATA32 *src, DATA32 *dst, int len);

static void evas_common_copy_rev_pixels_c           (DATA32 *src, DATA32 *dst, int len);


EAPI void
evas_common_blit_init(void)
{
}

EAPI void
evas_common_blit_rectangle(const RGBA_Image *src, RGBA_Image *dst, int src_x, int src_y, int w, int h, int dst_x, int dst_y)
{
   int y;
   Gfx_Func_Copy func;
   DATA32 *src_ptr, *dst_ptr;

   if ((!src->image.data) || (!dst->image.data)) return;
   /* clip clip clip */
   if (w <= 0) return;
   if (src_x + w > (int)src->cache_entry.w) w = src->cache_entry.w - src_x;
   if (w <= 0) return;
   if (src_x < 0)
     {
	dst_x -= src_x;
	w += src_x;
	src_x = 0;
     }
   if (w <= 0) return;

   if (h <= 0) return;
   if (src_y + h > (int)src->cache_entry.h) h = src->cache_entry.h - src_y;
   if (h <= 0) return;
   if (src_y < 0)
     {
	dst_y -= src_y;
	h += src_y;
	src_y = 0;
     }
   if (h <= 0) return;

   if (dst_x + w > (int)dst->cache_entry.w) w = dst->cache_entry.w - dst_x;
   if (w <= 0) return;
   if (dst_x < 0)
     {
	src_x -= dst_x;
	w += dst_x;
	dst_x = 0;
     }
   if (w <= 0) return;

   if (dst_y + h > (int)dst->cache_entry.h) h = dst->cache_entry.h - dst_y;
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
	if (((src_y * src->cache_entry.w) + src_x) > ((dst_y * dst->cache_entry.w) + dst_x))
	  {
	     func = evas_common_draw_func_copy_get(w, 0);
	     for (y = 0; y < h; y++)
	       {
		  src_ptr = src->image.data + ((y + src_y) * src->cache_entry.w) + src_x;
		  dst_ptr = dst->image.data + ((y + dst_y) * dst->cache_entry.w) + dst_x;
		  func(src_ptr, dst_ptr, w);
	       }
	  }
	/* reverse */
	else
	  {
	     func = evas_common_draw_func_copy_get(w, 1);
	     for (y = h - 1; y >= 0; y--)
	       {
		  src_ptr = src->image.data + ((y + src_y) * src->cache_entry.w) + src_x;
		  dst_ptr = dst->image.data + ((y + dst_y) * dst->cache_entry.w) + dst_x;
		  func(src_ptr, dst_ptr, w);
	       }
	  }
     }
   else
     {
	func = evas_common_draw_func_copy_get(w, 0);
	for (y = 0; y < h; y++)
	  {
	     src_ptr = src->image.data + ((y + src_y) * src->cache_entry.w) + src_x;
	     dst_ptr = dst->image.data + ((y + dst_y) * dst->cache_entry.w) + dst_x;
	     func(src_ptr, dst_ptr, w);
	  }
     }
}

/****************************************************************************/

static void
evas_common_copy_rev_pixels_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;

   src += len - 1;
   while (dst < dst_end) *dst++ = *src--;
}


#ifdef BUILD_NEON
static void
evas_common_copy_pixels_rev_neon(DATA32 *src, DATA32 *dst, int len)
{
   uint32_t *tmp = (void *)37;
#define AP	"evas_common_copy_rev_pixels_neon_"
   asm volatile (
		".fpu neon				\n\t"
		// Can we do 32 byte?
		"andS		%[tmp],	%[d], $0x1f	\n\t"
		"beq		"AP"quadstart		\n\t"

		// Can we do at least 16 byte?
		"andS		%[tmp], %[d], $0x4	\n\t"
		"beq		"AP"dualstart		\n\t"

	// Only once
	AP"singleloop:					\n\t"
		"sub		%[s], #4		\n\t"
		"vld1.32	d0[0],  [%[s]]		\n\t"
		"vst1.32	d0[0],  [%[d]]!		\n\t"

	// Up to 3 times
	AP"dualstart:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #31		\n\t"
		"blt		"AP"loopout		\n\t"

		"andS		%[tmp], %[d], $0x1f	\n\t"
		"beq		"AP"quadstart		\n\t"

	AP"dualloop:					\n\t"
		"sub		%[s], #8		\n\t"
		"vldm		%[s], {d0}		\n\t"
		"vrev64.32	d1, d0			\n\t"
		"vstm		%[d]!, {d1}		\n\t"

		"andS		%[tmp], %[d], $0x1f	\n\t"
		"bne		"AP"dualloop		\n\t"


	AP"quadstart:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"blt		"AP"loopout		\n\t"

		"sub		%[tmp],%[e],#32		\n\t"

	AP "quadloop:					\n\t"
		"sub		%[s],	#32		\n\t"
		"vldm		%[s],	{d0,d1,d2,d3}	\n\t"

		"vrev64.32	d7,d0			\n\t"
		"vrev64.32	d6,d1			\n\t"
		"vrev64.32	d5,d2			\n\t"
		"vrev64.32	d4,d3			\n\t"

		"vstm		%[d]!,	{d4,d5,d6,d7}	\n\t"

		"cmp		%[tmp], %[d]		\n\t"
                "bhi		"AP"quadloop		\n\t"


	AP "loopout:					\n\t"
		"cmp 		%[d], %[e]		\n\t"
                "beq 		"AP"done		\n\t"
		"sub		%[tmp],%[e], %[d]	\n\t"
		"cmp		%[tmp],$0x04		\n\t"
		"beq		"AP"singleloop2		\n\t"

	AP "dualloop2:					\n\t"
		"sub		%[tmp],%[e],$0x7	\n\t"
	AP "dualloop2int:				\n\t"
		"sub		%[s], #8		\n\t"
		"vldm		%[s], {d0}		\n\t"
		"vrev64.32	d1,d0			\n\t"
		"vstm		%[d]!, {d1}		\n\t"

		"cmp 		%[tmp], %[d]		\n\t"
		"bhi 		"AP"dualloop2int	\n\t"

		// Single ??
		"cmp 		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP "singleloop2:				\n\t"
		"sub		%[s], #4		\n\t"
		"vld1.32	d0[0], [%[s]]		\n\t"
		"vst1.32	d0[0], [%[d]]		\n\t"

	AP "done:\n\t"

		: // No output regs
		// Input
		: [s] "r" (src + len), [e] "r" (dst + len), [d] "r" (dst),[tmp] "r" (tmp)
		// Clobbered
		: "q0","q1","q2","q3","0","1","memory"
   );
#undef AP

}
#endif


static void
evas_common_copy_pixels_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end = dst + len;
   
   while (dst < dst_end) *dst++ = *src++;
}

#ifdef BUILD_MMX
static void
evas_common_copy_pixels_mmx(DATA32 *src, DATA32 *dst, int len)
{ // XXX cppcheck: [./src/lib/engines/common/evas_blit_main.c:248]: (error) Invalid number of character ({). Can't process file.
  // so... wtf? what's wrong with this { ? or anytrhing surrounding it?
   DATA32 *dst_end, *dst_end_pre;
#ifdef ALIGN_FIX
   intptr_t src_align;
   intptr_t dst_align;
   
   src_align = (intptr_t)src & 0x3f; /* 64 byte alignment */
   dst_align = (intptr_t)dst & 0x3f; /* 64 byte alignment */
   
   if ((src_align != dst_align) ||
       ((src_align & 0x3) != 0))
     {
	evas_common_copy_pixels_c(src, dst, len);
	return;
     }

   while ((src_align > 0) && (len > 0))
     {
	*dst++ = *src++;
	len--;
	src_align -= sizeof(DATA32);
     }
#endif /* ALIGN_FIX */

   dst_end = dst + len;
   dst_end_pre = dst + ((len / 16) * 16);

   while (dst < dst_end_pre)
     {
	MOVE_16DWORDS_MMX(src, dst);
	src += 16;
	dst += 16;
     }
   while (dst < dst_end) *dst++ = *src++;
}
#endif

#ifdef BUILD_MMX
static void
evas_common_copy_pixels_mmx2(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end, *dst_end_pre;
#ifdef ALIGN_FIX
   intptr_t src_align;
   intptr_t dst_align;

   src_align = (intptr_t)src & 0x3f; /* 64 byte alignment */
   dst_align = (intptr_t)dst & 0x3f; /* 64 byte alignment */

   if ((src_align != dst_align) ||
       ((src_align & 0x3) != 0))
     {
	evas_common_copy_pixels_c(src, dst, len);
	return;
     }

   while ((src_align > 0) && (len > 0))
     {
	*dst++ = *src++;
	len--;
	src_align -= sizeof(DATA32);
     }
#endif

   dst_end = dst + len;
   dst_end_pre = dst + ((len / 16) * 16);

   while (dst < dst_end_pre)
     {
	MOVE_16DWORDS_MMX(src, dst);
	src += 16;
	dst += 16;
     }
   while (dst < dst_end) *dst++ = *src++;
}
#endif

#ifdef BUILD_NEON
static void
evas_common_copy_pixels_neon(DATA32 *src, DATA32 *dst, int len){
   uint32_t *e,*tmp = (void *)37;
   e = dst + len;
#define AP	"evas_common_copy_pixels_neon_"
   asm volatile (
		".fpu neon				\n\t"
		// Can we do 32 byte?
		"andS		%[tmp],	%[d], $0x1f	\n\t"
		"beq		"AP"quadstart		\n\t"

		// Can we do at least 16 byte?
		"andS		%[tmp], %[d], $0x4	\n\t"
		"beq		"AP"dualstart		\n\t"

	// Only once
	AP"singleloop:					\n\t"
		"vld1.32	d0[0],  [%[s]]!		\n\t"
		"vst1.32	d0[0],  [%[d]]!		\n\t"

	// Up to 3 times
	AP"dualstart:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #31		\n\t"
		"blt		"AP"loopout		\n\t"

		"andS		%[tmp], %[d], $0x1f	\n\t"
		"beq		"AP"quadstart		\n\t"

	AP"dualloop:					\n\t"
		"vldm		%[s]!, {d0}		\n\t"
		"vstm		%[d]!, {d0}		\n\t"

		"andS		%[tmp], %[d], $0x1f	\n\t"
		"bne		"AP"dualloop		\n\t"


	AP"quadstart:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #64		\n\t"
		"blt		"AP"loopout		\n\t"

		"sub		%[tmp],%[e],#63		\n\t"

	AP "quadloop:					\n\t"
		"vldm		%[s]!,	{d0,d1,d2,d3}	\n\t"
		"vldm		%[s]!,	{d4,d5,d6,d7}	\n\t"
		"vstm		%[d]!,	{d0,d1,d2,d3}	\n\t"
		"vstm		%[d]!,	{d4,d5,d6,d7}	\n\t"

		"cmp		%[tmp], %[d]		\n\t"
                "bhi		"AP"quadloop		\n\t"


	AP "loopout:					\n\t"
		"cmp 		%[d], %[e]		\n\t"
                "beq 		"AP"done		\n\t"
		"sub		%[tmp],%[e], %[d]	\n\t"
		"cmp		%[tmp],$0x04		\n\t"
		"beq		"AP"singleloop2		\n\t"

	AP "dualloop2:					\n\t"
		"sub		%[tmp],%[e],$0x7	\n\t"
	AP "dualloop2int:				\n\t"
		"vldm		%[s]!, {d0}		\n\t"
		"vstm		%[d]!, {d0}		\n\t"

		"cmp 		%[tmp], %[d]		\n\t"
		"bhi 		"AP"dualloop2int	\n\t"

		// Single ??
		"cmp 		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP "singleloop2:				\n\t"
		"vld1.32	d0[0], [%[s]]		\n\t"
		"vst1.32	d0[0], [%[d]]		\n\t"

	AP "done:\n\t"

		: // No output regs
		// Input
		: [s] "r" (src), [e] "r" (e), [d] "r" (dst),[tmp] "r" (tmp)
		// Clobbered
		: "q0","q1","q2","q3","memory"
   );
#undef AP

}
#endif /* BUILD_NEON */

#ifdef BUILD_MMX
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
	evas_common_copy_pixels_c(src, dst, len);
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

static void
evas_common_copy_pixels_rev_c(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end;

   src = src + len - 1;
   dst_end = dst - 1;
   dst = dst + len - 1;

   while (dst > dst_end) *dst-- = *src--;
}

#ifdef BUILD_MMX
static void
evas_common_copy_pixels_rev_mmx(DATA32 *src, DATA32 *dst, int len)
{
   DATA32 *dst_end, *dst_end_pre;

   if (len >= 16)
     {
	src = src + len - 16;
	dst_end = dst;
	dst_end_pre = dst + len - ((len / 16) * 16);
	dst = dst + len - 16;

	while (dst >= dst_end_pre)
	  {
	     MOVE_16DWORDS_MMX(src, dst);
	     src -= 16;
	     dst -= 16;
	  }
	src += 15;
	dst += 15;
	while (dst >= dst_end)
	     *dst-- = *src--;
     }
   else
     {
	src = src + len - 1;
	dst_end = dst - 1;
	dst = dst + len - 1;
	while (dst > dst_end)
	     *dst-- = *src--;
     }
}
#endif

#ifdef BUILD_MMX
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
	     src_ptr -= 16;
	     dst_ptr -= 16;
	  }
	src_ptr += 15;
	dst_ptr += 15;
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
   if (reverse == -1)
	return evas_common_copy_rev_pixels_c;
   if (reverse)
     {
#ifdef BUILD_MMX
	if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 64 * 64))
	  return evas_common_copy_pixels_rev_sse;
	else if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
          return evas_common_copy_pixels_rev_mmx;
#endif
#ifdef BUILD_NEON
        if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
          return evas_common_copy_pixels_rev_neon;
#endif
        return evas_common_copy_pixels_rev_c;
     }
   else
     {
#ifdef BUILD_MMX
        if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 64 * 64))
          return evas_common_copy_pixels_sse;
        else if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
          return evas_common_copy_pixels_mmx2;
        else if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
          return evas_common_copy_pixels_mmx;
#endif
#ifdef BUILD_NEON
        if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
          return evas_common_copy_pixels_neon;
#endif
     }
   return evas_common_copy_pixels_c;
}
