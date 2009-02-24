/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_convert_color.h"
#include "evas_scale_span.h"

static void
evas_common_scale_rgba_span_(DATA32 *src, DATA8 *mask __UNUSED__, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   int  mul = 0, step = 1;
   DATA32 *pdst = dst;

   if (!src || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (mul_col != 0xffffffff)
	mul = 1;
   if (dir < 0)
     {
	pdst += dst_len - 1;
	step = -1;
     }

   if ((src_len == 1) || (dst_len == 1))
     {
	DATA32 c = *src;

	if (mul) c = MUL4_SYM(mul_col, c);
	while (dst_len--)
	   *dst++ = c;
	return;
     }

   if (src_len == dst_len)
     {
	if (mul)
	  {
#ifdef BUILD_MMX
	    pxor_r2r(mm0, mm0);
	    MOV_A2R(ALPHA_255, mm5)
	    MOV_P2R(mul_col, mm7, mm0)
#endif
	    while (dst_len--)
	      {
#ifdef BUILD_MMX
		MOV_P2R(*src, mm1, mm0)
		MUL4_SYM_R2R(mm7, mm1, mm5)
		MOV_R2P(mm1, *pdst, mm0)
#else
		*pdst = MUL4_SYM(mul_col, *src);
#endif
		src++;  pdst += step;
	      }
	    return;
	  }
	while (dst_len--)
	  {
	    *pdst = *src;
	    src++;  pdst += step;
	  }
	return;
     }

     {
	DATA32  dsxx = (((src_len - 1) << 16) / (dst_len - 1));
	DATA32  sxx = 0;
	int     sx = sxx >> 16;

#ifdef BUILD_MMX
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_255, mm5)
	if (mul)
	  {
	    MOV_P2R(mul_col, mm7, mm0)
	  }
#endif
	while (dst_len--)
	  {
	    DATA32   p2, p1 = 0;
	    int      a;

	    sx = (sxx >> 16);
	    if (sx < src_len)
		p1 = *(src + sx);
	    p2 = p1;
	    if ((sx + 1) < src_len)
	        p2 = *(src + sx + 1);
	    a = 1 + ((sxx - (sx << 16)) >> 8);
#ifdef BUILD_MMX
	    MOV_A2R(a, mm3)
	    MOV_P2R(p1, mm1, mm0)
	    MOV_P2R(p2, mm2, mm0)
	    INTERP_256_R2R(mm3, mm2, mm1, mm5)
	    if (mul)
	      {
		MUL4_SYM_R2R(mm7, mm1, mm5)
	      }
	    MOV_R2P(mm1, *pdst, mm0)
#else
	    p1 = INTERP_256(a, p2, p1);
	    if (mul)
		p1 = MUL4_SYM(mul_col, p1);
	    *pdst = p1;
#endif
	    pdst += step;  sxx += dsxx;
	  }
	return;
     }
}

static void
evas_common_scale_rgba_a8_span_(DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   int  mul = 0, step = 1;
   DATA32 *pdst = dst;

   if (!src || !mask || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (mul_col != 0xffffffff)
	mul = 1;
   if (dir < 0)
     {
	pdst += dst_len - 1;
	step = -1;
     }

   if ((src_len == 1) || (dst_len == 1))
     {
	DATA32 c = MUL_SYM(*mask, *src);

	if (mul) c = MUL4_SYM(mul_col, c);
	while (dst_len--)
	   *dst++ = c;
	return;
     }

   if (src_len == dst_len)
     {
#ifdef BUILD_MMX
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_255, mm5)
#endif
	if (mul)
	  {
#ifdef BUILD_MMX
	    MOV_P2R(mul_col, mm7, mm0)
#endif
	    while (dst_len--)
	      {
#ifdef BUILD_MMX
		MOV_P2R(*src, mm1, mm0)
		MOV_A2R(*mask, mm3)
		MUL4_SYM_R2R(mm3, mm1, mm5)
		MUL4_SYM_R2R(mm7, mm1, mm5)
		MOV_R2P(mm1, *pdst, mm0)
#else
		DATA32  c = MUL_SYM(*mask, *src);
		*pdst = MUL4_SYM(mul_col, c);
#endif
		src++;  mask++;  pdst += step;
	      }
	    return;
	  }
	while (dst_len--)
	  {
#ifdef BUILD_MMX
	    MOV_P2R(*src, mm1, mm0)
	    MOV_A2R(*mask, mm3)
	    MUL4_SYM_R2R(mm3, mm1, mm5)
	    MOV_R2P(mm1, *pdst, mm0)
#else
	    *pdst = MUL_SYM(*mask, *src);
#endif
	    src++;  mask++;  pdst += step;
	  }
	return;
     }

     {
	DATA32  dsxx = (((src_len - 1) << 16) / (dst_len - 1));
	DATA32  sxx = 0;
	int     sx = sxx >> 16;

#ifdef BUILD_MMX
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_255, mm5)
	if (mul)
	  {
	    MOV_P2R(mul_col, mm7, mm0)
	  }
#endif
	while (dst_len--)
	  {
	    DATA32   p2, p1 = 0;
	    int      a, a2, a1 = 0;

	    sx = (sxx >> 16);
	    if (sx < src_len)
	      {
		p1 = *(src + sx);
		a1 = *(mask + sx);
	      }
	    p2 = p1;  a2 = a1;
	    if ((sx + 1) < src_len)
	      {
		p2 = *(src + sx + 1);
		a2 = *(mask + sx + 1);
	      }
	    a = 1 + ((sxx - (sx << 16)) >> 8);
#ifdef BUILD_MMX
	    MOV_A2R(a, mm3)
	    MOV_P2R(p1, mm1, mm0)
	    MOV_P2R(p2, mm2, mm0)
	    INTERP_256_R2R(mm3, mm2, mm1, mm5)
	    a1 += 1 + ((a * (a2 - a1)) >> 8);
	    MOV_A2R(a1, mm3)
	    MUL4_256_R2R(mm3, mm1)
	    if (mul)
	      {
		MUL4_SYM_R2R(mm7, mm1, mm5)
	      }
	    MOV_R2P(mm1, *pdst, mm0)
#else
	    p1 = INTERP_256(a, p2, p1);
	    a1 += 1 + ((a * (a2 - a1)) >> 8);
	    p1 = MUL_256(a1, p1);
	    if (mul)
		p1 = MUL4_SYM(mul_col, p1);
	    *pdst = p1;
#endif
	    pdst += step;  sxx += dsxx;
	  }
	return;
     }
}

static void
evas_common_scale_a8_span_(DATA32 *src __UNUSED__, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   int    step = 1;
   DATA32 *pdst = dst;

   if (!mask || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (dir < 0)
     {
	pdst += dst_len - 1;
	step = -1;
     }

   if ((src_len == 1) || (dst_len == 1))
     {
	DATA32 c = MUL_SYM(*mask, mul_col);

	while (dst_len--)
	   *dst++ = c;
	return;
     }

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_P2R(mul_col, mm7, mm0)
#endif
   if (src_len == dst_len)
     {
	while (dst_len--)
	  {
#ifdef BUILD_MMX
	    MOV_A2R(*mask, mm3)
	    MUL4_SYM_R2R(mm7, mm3, mm5)
	    MOV_R2P(mm3, *pdst, mm0)
#else
	    *pdst = MUL_SYM(*mask, mul_col);
#endif
	    mask++;  pdst += step;
	  }
	return;
     }

     {
	DATA32  dsxx = (((src_len - 1) << 16) / (dst_len - 1));
	DATA32  sxx = 0;
	int     sx = sxx >> 16;

	while (dst_len--)
	  {
	    int   a, a2, a1 = 0;

	    sx = (sxx >> 16);
	    if (sx < src_len)
		a1 = *(mask + sx);
	    a2 = a1;
	    if ((sx + 1) < src_len)
		a2 = *(mask + sx + 1);
	    a = 1 + ((sxx - (sx << 16)) >> 8);
	    a1 += 1 + ((a * (a2 - a1)) >> 8);
#ifdef BUILD_MMX
	    MOV_A2R(a1, mm3)
	    MUL4_256_R2R(mm7, mm3)
	    MOV_R2P(mm3, *pdst, mm0)
#else
	    *pdst = MUL_256(a1, mul_col);
#endif
	    pdst += step;  sxx += dsxx;
	  }
	return;
     }
}

static void
evas_common_scale_clip_a8_span_(DATA32 *src __UNUSED__, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   int   mul = 0, step = 1;
   DATA32 *pdst = dst;

   if (!mask || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (mul_col != 0xffffffff)
	mul = 1;
   if (dir < 0)
     {
	pdst += dst_len - 1;
	step = -1;
     }

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   if (mul)
     {
	MOV_P2R(mul_col, mm7, mm0)
     }
#endif
   if ((src_len == 1) || (dst_len == 1))
     {
#ifdef BUILD_MMX
	MOV_A2R(*mask, mm3)
#else
	DATA32 c = *mask;
#endif
	if (mul)
	  {
#ifdef BUILD_MMX
	    MUL4_SYM_R2R(mm7, mm3, mm5)
#else
	    c = MUL_SYM(c, mul_col);
#endif
	    while (dst_len--)
	      {
#ifdef BUILD_MMX
		MOV_P2R(*dst, mm1, mm0)
		MUL4_SYM_R2R(mm3, mm1, mm5)
		MOV_R2P(mm1, *dst, mm0)
#else
		*dst = MUL4_SYM(c, *dst);
#endif
		dst++;
	      }
	    return;
	  }
	while (dst_len--)
	  {
#ifdef BUILD_MMX
	    MOV_P2R(*dst, mm1, mm0)
	    MUL4_SYM_R2R(mm3, mm1, mm5)
	    MOV_R2P(mm1, *dst, mm0)
#else
	    *dst = MUL_SYM(c, *dst);
#endif
	    dst++;
	  }
	return;
     }

   if (src_len == dst_len)
     {
	if (mul)
	  {
	    while (dst_len--)
	      {
#ifdef BUILD_MMX
		MOV_A2R(*mask, mm3)
		MUL4_SYM_R2R(mm7, mm3, mm5)
		MOV_P2R(*pdst, mm1, mm0)
		MUL4_SYM_R2R(mm3, mm1, mm5)
		MOV_R2P(mm1, *pdst, mm0)
#else
		DATA32 c = MUL_SYM(*mask, mul_col);

		*pdst = MUL4_SYM(c, *pdst);
#endif
		mask++;  pdst += step;
	      }
	    return;
	  }
	while (dst_len--)
	  {
#ifdef BUILD_MMX
	    MOV_A2R(*mask, mm3)
	    MOV_P2R(*pdst, mm1, mm0)
	    MUL4_SYM_R2R(mm3, mm1, mm5)
	    MOV_R2P(mm1, *pdst, mm0)
#else
	    *pdst = MUL_SYM(*mask, *pdst);
#endif
	    mask++;  pdst += step;
	  }
	return;
     }

     {
	DATA32  dsxx = (((src_len - 1) << 16) / (dst_len - 1));
	DATA32  sxx = 0;
	int     sx = sxx >> 16;

	while (dst_len--)
	  {
	    int   a, a2, a1 = 0;

	    sx = (sxx >> 16);
	    if (sx < src_len)
		a1 = *(mask + sx);
	    a2 = a1;
	    if ((sx + 1) < src_len)
		a2 = *(mask + sx + 1);
	    a = 1 + ((sxx - (sx << 16)) >> 8);
	    a1 += 1 + ((a * (a2 - a1)) >> 8);
#ifdef BUILD_MMX
	    MOV_A2R(a1, mm3)
	    MOV_P2R(*pdst, mm1, mm0)
	    MUL4_256_R2R(mm3, mm1)
	    if (mul)
	      {
		MUL4_SYM_R2R(mm7, mm1, mm5)
	      }
	    MOV_R2P(mm1, *pdst, mm0)
#else
	    *pdst = MUL_256(a1, *pdst);
	    if (mul)
		*pdst = MUL4_SYM(mul_col, *pdst);
#endif
	    pdst += step;  sxx += dsxx;
	  }
	return;
     }
}

EAPI void
evas_common_scale_rgba_span(DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   evas_common_scale_rgba_span_(src, mask, src_len, mul_col, dst, dst_len, dir);
   evas_common_cpu_end_opt();
}

EAPI void
evas_common_scale_rgba_a8_span(DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   evas_common_scale_rgba_a8_span_(src, mask, src_len, mul_col, dst, dst_len, dir);
   evas_common_cpu_end_opt();
}

EAPI void
evas_common_scale_a8_span(DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   evas_common_scale_a8_span_(src, mask, src_len, mul_col, dst, dst_len, dir);
   evas_common_cpu_end_opt();
}

EAPI void
evas_common_scale_clip_a8_span(DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   evas_common_scale_clip_a8_span_(src, mask, src_len, mul_col, dst, dst_len, dir);
   evas_common_cpu_end_opt();
}

EAPI void
evas_common_scale_hsva_span(DATA32 *src, DATA8 *mask __UNUSED__, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   int  mul = 0, step = 1;
   DATA32 *pdst = dst;

   if (!src || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (mul_col != 0xffffffff)
	mul = 1;
   if (dir < 0)
     {
	pdst += dst_len - 1;
	step = -1;
     }

   if ((src_len == 1) || (dst_len == 1))
     {
	DATA32 c = *src;

	if (mul) c = MUL4_SYM(mul_col, c);
	while (dst_len--)
	   *dst++ = c;
	return;
     }

   if (src_len == dst_len)
     {
	if (mul)
	  {
	    while (dst_len--)
	      {
		*pdst = MUL4_SYM(mul_col, *src);
		src++;  pdst += step;
	      }
	    return;
	  }
	while (dst_len--)
	  {
	    *pdst = *src;
	    src++;  pdst += step;
	  }
	return;
     }

     {
	DATA32  dsxx = (((src_len - 1) << 16) / (dst_len - 1));
	DATA32  sxx = 0;
	int     sx = sxx >> 16;

	while (dst_len--)
	  {
	    DATA32   p2, p1 = 0;
	    int      a, h1, s1, v1, h2, s2, v2;

	    sx = (sxx >> 16);
	    if (sx < src_len)
		p1 = *(src + sx);
	    evas_common_convert_color_rgb_to_hsv_int((p1 >> 16) & 0xff, (p1 >> 8) & 0xff, p1 & 0xff,
						     &h1, &s1, &v1);
	    p2 = p1;
	    if ((sx + 1) < src_len)
	        p2 = *(src + sx + 1);
	    evas_common_convert_color_rgb_to_hsv_int((p2 >> 16) & 0xff, (p2 >> 8) & 0xff, p2 & 0xff,
						     &h2, &s2, &v2);
	    a = 1 + ((sxx - (sx << 16)) >> 8);
	    h1 += (a * (h2 - h1)) >> 8;
	    s1 += (a * (s2 - s1)) >> 8;
	    v1 += (a * (v2 - v1)) >> 8;
	    a = (((((p2 >> 8) & 0xff0000) - ((p1 >> 8) & 0xff0000)) * a) +
	         (p1 & 0xff000000)) & 0xff000000;
	    evas_common_convert_color_hsv_to_rgb_int(h1, s1, v1, &h2, &s2, &v2);
	    p1 = a + RGB_JOIN(h2,s2,v2);
	    if (mul)
		p1 = MUL4_SYM(mul_col, p1);
	    *pdst = p1;
	    pdst += step;  sxx += dsxx;
	  }
	return;
     }
}

EAPI void
evas_common_scale_hsva_a8_span(DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir)
{
   int  mul = 0, step = 1;
   DATA32 *pdst = dst;

   if (!src || !mask || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (mul_col != 0xffffffff)
	mul = 1;
   if (dir < 0)
     {
	pdst += dst_len - 1;
	step = -1;
     }

   if ((src_len == 1) || (dst_len == 1))
     {
	DATA32 c = MUL_SYM(*mask, *src);

	if (mul) c = MUL4_SYM(mul_col, c);
	while (dst_len--)
	   *dst++ = c;
	return;
     }

   if (src_len == dst_len)
     {
	if (mul)
	  {
	    while (dst_len--)
	      {
		DATA32  c = MUL_SYM(*mask, *src);
		*pdst = MUL4_SYM(mul_col, c);
		src++;  mask++;  pdst += step;
	      }
	    return;
	  }
	while (dst_len--)
	  {
	    *pdst = MUL_SYM(*mask, *src);
	    src++;  mask++;  pdst += step;
	  }
	return;
     }

     {
	DATA32  dsxx = (((src_len - 1) << 16) / (dst_len - 1));
	DATA32  sxx = 0;
	int     sx = sxx >> 16;

	while (dst_len--)
	  {
	    DATA32   p2, p1 = 0;
	    int      a, a2, a1 = 0;
	    int      h1, s1, v1, h2, s2, v2;

	    sx = (sxx >> 16);
	    if (sx < src_len)
	      {
		p1 = *(src + sx);
		a1 = *(mask + sx);
	      }
	    p2 = p1;  a2 = a1;
	    if ((sx + 1) < src_len)
	      {
		p2 = *(src + sx + 1);
		a2 = *(mask + sx + 1);
	      }
	    evas_common_convert_color_rgb_to_hsv_int((p1 >> 16) & 0xff, (p1 >> 8) & 0xff, p1 & 0xff,
						      &h1, &s1, &v1);
	    evas_common_convert_color_rgb_to_hsv_int((p2 >> 16) & 0xff, (p2 >> 8) & 0xff, p2 & 0xff,
						      &h2, &s2, &v2);
	    a = 1 + ((sxx - (sx << 16)) >> 8);
	    a1 += (a * (a2 - a1)) >> 8;
	    h1 += (a * (h2 - h1)) >> 8;
	    s1 += (a * (s2 - s1)) >> 8;
	    v1 += (a * (v2 - v1)) >> 8;
	    a = (((((p2 >> 8) & 0xff0000) - ((p1 >> 8) & 0xff0000)) * a) +
	         (p1 & 0xff000000)) & 0xff000000;

	    evas_common_convert_color_hsv_to_rgb_int(h1, s1, v1, &h2, &s2, &v2);
	    p1 = a + RGB_JOIN(h2,s2,v2);
	    p1 = MUL_SYM(a1, p1);
	    if (mul)
		p1 = MUL4_SYM(mul_col, p1);
	    *pdst = p1;
	    pdst += step;  sxx += dsxx;
	  }
	return;
     }
}
