#include "evas_common.h"


static DATA32 **scale_calc_y_points(DATA32 *src, int sw, int sh, int dh);
static int     *scale_calc_x_points(int sw, int dw);
static int     *scale_calc_a_points(int s, int d);

static DATA32 **
scale_calc_y_points(DATA32 *src, int sw, int sh, int dh)
{
   DATA32 **p;
   int i, val, inc;

   p = malloc((dh + 1) * sizeof(DATA32 *));
   if (!p) return NULL;
   val = 0;
   inc = (sh << 16) / dh;
   for (i = 0; i < dh; i++)
     {
	p[i] = src + ((val >> 16) * sw);
	val += inc;
     }
   p[i] = p[i - 1];
   return p;
}

static int *
scale_calc_x_points(int sw, int dw)
{
   int *p;
   int i, val, inc;

   p = malloc((dw + 1) * sizeof(int));
   if (!p) return NULL;
   val = 0;
   inc = (sw << 16) / dw;
   for (i = 0; i < dw; i++)
     {
	p[i] = val >> 16;
	val += inc;
     }
   p[i] = p[i - 1];
   return p;
}

static int *
scale_calc_a_points(int s, int d)
{
   int *p;
   int i, val, inc;

   p = malloc(d * sizeof(int));
   if (!p) return NULL;
   if (d >= s)
     {
	val = 0;
	inc = (s << 16) / d;
	for (i = 0; i < d; i++)
	  {
	     p[i] = (val >> 8) - ((val >> 8) & 0xffffff00);
	     if ((val >> 16) >= (s - 1)) p[i] = 0;
	     val += inc;
	  }
     }
   else
     {
	int ap, Cp;

	val = 0;
	inc = (s << 16) / d;
	Cp = ((d << 14) / s) + 1;
	for (i = 0; i < d; i++)
	  {
	     ap = ((0x100 - ((val >> 8) & 0xff)) * Cp) >> 8;
	     p[i] = ap | (Cp << 16);
	     val += inc;
	  }
     }
//   sleep(1);
   return p;
}

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_C
void
evas_common_scale_rgba_mipmap_down_2x2_c(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *src_ptr2, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   src_ptr2 = src + src_w;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	src_ptr = src + (y * src_w * 2);
	src_ptr2 = src_ptr + src_w;
	for (x = 0; x < dst_w; x++)
	  {
	     R_VAL(dst_ptr) = (R_VAL(src_ptr) + R_VAL(src_ptr + 1) + R_VAL(src_ptr2) + R_VAL(src_ptr2 + 1)) >> 2;
	     G_VAL(dst_ptr) = (G_VAL(src_ptr) + G_VAL(src_ptr + 1) + G_VAL(src_ptr2) + G_VAL(src_ptr2 + 1)) >> 2;
	     B_VAL(dst_ptr) = (B_VAL(src_ptr) + B_VAL(src_ptr + 1) + B_VAL(src_ptr2) + B_VAL(src_ptr2 + 1)) >> 2;
	     A_VAL(dst_ptr) = (A_VAL(src_ptr) + A_VAL(src_ptr + 1) + A_VAL(src_ptr2) + A_VAL(src_ptr2 + 1)) >> 2;

	     src_ptr+=2;
	     src_ptr2+=2;
	     dst_ptr++;
	  }
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_C
void
evas_common_scale_rgba_mipmap_down_2x1_c(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	src_ptr = src + (y * src_w * 2);
	for (x = 0; x < dst_w; x++)
	  {
	     R_VAL(dst_ptr) = (R_VAL(src_ptr) + R_VAL(src_ptr + 1)) >> 1;
	     G_VAL(dst_ptr) = (G_VAL(src_ptr) + G_VAL(src_ptr + 1)) >> 1;
	     B_VAL(dst_ptr) = (B_VAL(src_ptr) + B_VAL(src_ptr + 1)) >> 1;
	     A_VAL(dst_ptr) = (A_VAL(src_ptr) + A_VAL(src_ptr + 1)) >> 1;

	     src_ptr+=2;
	     dst_ptr++;
	  }
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_C
void
evas_common_scale_rgba_mipmap_down_1x2_c(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *src_ptr2, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   src_ptr2 = src + src_w;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	src_ptr = src + (y * src_w * 2);
	src_ptr2 = src_ptr + src_w;
	for (x = 0; x < dst_w; x++)
	  {
	     R_VAL(dst_ptr) = (R_VAL(src_ptr) + R_VAL(src_ptr2)) >> 1;
	     G_VAL(dst_ptr) = (G_VAL(src_ptr) + G_VAL(src_ptr2)) >> 1;
	     B_VAL(dst_ptr) = (B_VAL(src_ptr) + B_VAL(src_ptr2)) >> 1;
	     A_VAL(dst_ptr) = (A_VAL(src_ptr) + A_VAL(src_ptr2)) >> 1;

	     src_ptr+=2;
	     src_ptr2+=2;
	     dst_ptr++;
	  }
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_C
void
evas_common_scale_rgb_mipmap_down_2x2_c(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *src_ptr2, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   src_ptr2 = src + src_w;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	for (x = 0; x < dst_w; x++)
	  {
	     R_VAL(dst_ptr) = (R_VAL(src_ptr) + R_VAL(src_ptr + 1) + R_VAL(src_ptr2) + R_VAL(src_ptr2 + 1)) >> 2;
	     G_VAL(dst_ptr) = (G_VAL(src_ptr) + G_VAL(src_ptr + 1) + G_VAL(src_ptr2) + G_VAL(src_ptr2 + 1)) >> 2;
	     B_VAL(dst_ptr) = (B_VAL(src_ptr) + B_VAL(src_ptr + 1) + B_VAL(src_ptr2) + B_VAL(src_ptr2 + 1)) >> 2;
	     A_VAL(dst_ptr) = 0xff;

	     src_ptr+=2;
	     src_ptr2+=2;
	     dst_ptr++;
	  }
	src_ptr += src_w;
	src_ptr2 += src_w;
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_C
void
evas_common_scale_rgb_mipmap_down_2x1_c(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	for (x = 0; x < dst_w; x++)
	  {
	     R_VAL(dst_ptr) = (R_VAL(src_ptr) + R_VAL(src_ptr + 1)) >> 1;
	     G_VAL(dst_ptr) = (G_VAL(src_ptr) + G_VAL(src_ptr + 1)) >> 1;
	     B_VAL(dst_ptr) = (B_VAL(src_ptr) + B_VAL(src_ptr + 1)) >> 1;
	     A_VAL(dst_ptr) = 0xff;

	     src_ptr+=2;
	     dst_ptr++;
	  }
	src_ptr += src_w;
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_C
void
evas_common_scale_rgb_mipmap_down_1x2_c(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *src_ptr2, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   src_ptr2 = src + src_w;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	for (x = 0; x < dst_w; x++)
	  {
	     R_VAL(dst_ptr) = (R_VAL(src_ptr) + R_VAL(src_ptr2)) >> 1;
	     G_VAL(dst_ptr) = (G_VAL(src_ptr) + G_VAL(src_ptr2)) >> 1;
	     B_VAL(dst_ptr) = (B_VAL(src_ptr) + B_VAL(src_ptr2)) >> 1;
	     A_VAL(dst_ptr) = 0xff;

	     src_ptr+=2;
	     src_ptr2+=2;
	     dst_ptr++;
	  }
	src_ptr += src_w;
	src_ptr2 += src_w;
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_MMX
void
evas_common_scale_rgba_mipmap_down_2x2_mmx(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *src_ptr2, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   src_ptr2 = src + src_w;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	src_ptr = src + (y * src_w * 2);
	src_ptr2 = src_ptr + src_w;
	for (x = 0; x < dst_w; x++)
	  {
	     punpcklbw_m2r(src_ptr[0], mm0);
	     punpcklbw_m2r(src_ptr[1], mm1);
	     punpcklbw_m2r(src_ptr2[0], mm2);
	     punpcklbw_m2r(src_ptr2[1], mm3);
	     psrlw_i2r(8, mm0);
	     psrlw_i2r(8, mm1);
	     psrlw_i2r(8, mm2);
	     psrlw_i2r(8, mm3);
	     paddw_r2r(mm1, mm0);
	     paddw_r2r(mm2, mm0);
	     paddw_r2r(mm3, mm0);
	     psrlw_i2r(2, mm0);
	     packuswb_r2r(mm0, mm0);
	     movd_r2m(mm0, dst_ptr[0]);

	     src_ptr+=2;
	     src_ptr2+=2;
	     dst_ptr++;
	  }
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_MMX
void
evas_common_scale_rgba_mipmap_down_2x1_mmx(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	src_ptr = src + (y * src_w * 2);
	for (x = 0; x < dst_w; x++)
	  {
	     punpcklbw_m2r(src_ptr[0], mm0);
	     punpcklbw_m2r(src_ptr[1], mm1);
	     psrlw_i2r(8, mm0);
	     psrlw_i2r(8, mm1);
	     paddw_r2r(mm1, mm0);
	     psrlw_i2r(1, mm0);
	     packuswb_r2r(mm0, mm0);
	     movd_r2m(mm0, dst_ptr[0]);

	     src_ptr+=2;
	     dst_ptr++;
	  }
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_MMX
void
evas_common_scale_rgba_mipmap_down_1x2_mmx(DATA32 *src, DATA32 *dst, int src_w, int src_h)
{
   int x, y, dst_w, dst_h;
   DATA32 *src_ptr, *src_ptr2, *dst_ptr;

   dst_w = src_w >> 1;
   dst_h = src_h >> 1;

   if (dst_w < 1) dst_w = 1;
   if (dst_h < 1) dst_h = 1;

   src_ptr = src;
   src_ptr2 = src + src_w;
   dst_ptr = dst;
   for (y = 0; y < dst_h; y++)
     {
	src_ptr = src + (y * src_w * 2);
	src_ptr2 = src_ptr + src_w;
	for (x = 0; x < dst_w; x++)
	  {
	     punpcklbw_m2r(src_ptr[0], mm0);
	     punpcklbw_m2r(src_ptr2[0], mm1);
	     psrlw_i2r(8, mm0);
	     psrlw_i2r(8, mm1);
	     paddw_r2r(mm1, mm0);
	     psrlw_i2r(1, mm0);
	     packuswb_r2r(mm0, mm0);
	     movd_r2m(mm0, dst_ptr[0]);

	     src_ptr+=2;
	     src_ptr2+=2;
	     dst_ptr++;
	  }
     }
}
#endif
#endif

#ifdef BUILD_SCALE_SMOOTH
# ifdef BUILD_MMX
#  undef SCALE_FUNC
#  define SCALE_FUNC evas_common_scale_rgba_in_to_out_clip_smooth_mmx
#  undef SCALE_USING_MMX
#  define SCALE_USING_MMX
#  include "evas_scale_smooth_scaler.c"
# endif
# ifdef BUILD_C
#  undef SCALE_FUNC
#  define SCALE_FUNC evas_common_scale_rgba_in_to_out_clip_smooth_c
#  undef SCALE_USING_MMX
#  include "evas_scale_smooth_scaler.c"
# endif
void
evas_common_scale_rgba_in_to_out_clip_smooth(RGBA_Image *src, RGBA_Image *dst,
				 RGBA_Draw_Context *dc,
				 int src_region_x, int src_region_y,
				 int src_region_w, int src_region_h,
				 int dst_region_x, int dst_region_y,
				 int dst_region_w, int dst_region_h)
{
# ifdef BUILD_MMX
   int mmx, sse, sse2;
# endif
   int c, cx, cy, cw, ch;
   Cutout_Rect *rects, *r;
   Evas_Object_List *l;
   /* handle cutouts here! */

   if ((dst_region_w <= 0) || (dst_region_h <= 0)) return;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->image->w, dst->image->h)))
     return;
# ifdef BUILD_MMX
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
# endif
   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
# ifdef BUILD_MMX
	if (mmx)
	  evas_common_scale_rgba_in_to_out_clip_smooth_mmx(src, dst, dc,
					       src_region_x, src_region_y,
					       src_region_w, src_region_h,
					       dst_region_x, dst_region_y,
					       dst_region_w, dst_region_h);
	else
# endif
# ifdef BUILD_C
	  evas_common_scale_rgba_in_to_out_clip_smooth_c(src, dst, dc,
					     src_region_x, src_region_y,
					     src_region_w, src_region_h,
					     dst_region_x, dst_region_y,
					     dst_region_w, dst_region_h);
# endif
	return;
     }
   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->image->w, dst->image->h);
   evas_common_draw_context_clip_clip(dc, dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
	return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (l = (Evas_Object_List *)rects; l; l = l->next)
     {
	r = (Cutout_Rect *)l;
	evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
# ifdef BUILD_MMX
	if (mmx)
	  evas_common_scale_rgba_in_to_out_clip_smooth_mmx(src, dst, dc,
					       src_region_x, src_region_y,
					       src_region_w, src_region_h,
					       dst_region_x, dst_region_y,
					       dst_region_w, dst_region_h);
	else
# endif
# ifdef BUILD_C
	  evas_common_scale_rgba_in_to_out_clip_smooth_c(src, dst, dc,
					     src_region_x, src_region_y,
					     src_region_w, src_region_h,
					     dst_region_x, dst_region_y,
					     dst_region_w, dst_region_h);
# endif
     }
   evas_common_draw_context_apply_free_cutouts(rects);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}
#endif


#ifdef BUILD_C
static void
evas_common_scale_rgba_span_c(DATA32 *src, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len)
{
   int  mul = 0;

   if (!src || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (mul_col != 0xffffffff)
	mul = 1;

   if (src_len == dst_len)
     {
	if (!mul)
	    memcpy(dst, src, sizeof(DATA32) * src_len);
	else
	  {
	    DATA32 *dst_end = dst + dst_len;
	    while (dst < dst_end)
	      {
		*dst = MUL4_SYM(mul_col, *src);
		src++;  dst++;
	      }
	  }
	return;
     }

   if (src_len < dst_len)
     {
	DATA32  dsxx = ((src_len << 16) / dst_len);
	DATA32  sxx = 0;
	int     sx = sxx >> 16;
	DATA32  *dst_end = dst + dst_len;

	while (dst < dst_end)
	  {
	    DATA32   a, p2, p1 = 0;

	    sx = (sxx >> 16);
	    if (sx < src_len)
		p1 = *(src + sx);
	    p2 = p1;
	    if ((sx + 1) < src_len)
	        p2 = *(src + sx + 1);
	    a = 256 - ((sxx - (sx << 16)) >> 8);
	    p1 = INTERP_256(a, p2, p1);
	    if (mul)
		p1 = MUL4_SYM(mul_col, p1);
	    *dst++ = p1;  sxx += dsxx;
	  }
     }
   else
     {
	DATA32  dtxx = ((dst_len << 16) / src_len);
	DATA32  da = dtxx >> 8;
	DATA32  txx = 0, ntxx = txx;
	int     tx = txx >> 16, ntx, x = 0;
	DATA32  *dst_end = dst + dst_len;
	DATA32  *src_end = src + src_len;
	DATA32  q = 0;

	while (dst < dst_end)
	  {
	    DATA32  a, p = 0;

	    if (src < src_end)
		p = *src;
	    ntxx += dtxx;
	    ntx = ntxx >> 16;
	    if (ntx != tx)
	      {
		if (ntx > x)
		  {
		    a = 256 - ((txx - (tx << 16)) >> 8);
		    q += MUL_256(a, p);
		    if (mul)
			q = MUL4_SYM(mul_col, q);
		    *dst++ = q;
		    x++;
		  }
		a = 1 + ((ntxx - (ntx << 16)) >> 8);
		q = MUL_256(a, p);
		tx = ntx;
	      }
	    else if (tx == x)
		q += MUL_256(da, p);
	    src++;
	    txx = ntxx;
	  }
     }
}
#endif

#ifdef BUILD_MMX
static void
evas_common_scale_rgba_span_mmx(DATA32 *src, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len)
{
   int  mul = 0;

   if (!src || !dst) return;
   if ((src_len < 1) || (dst_len < 1)) return;
   if ((src_len > 65535) || (dst_len > 65535)) return;
   if (mul_col != 0xffffffff)
	mul = 1;

   if (src_len == dst_len)
     {
	if (!mul)
	    memcpy(dst, src, sizeof(DATA32) * src_len);
	else
	  {
	    DATA32 *dst_end = dst + dst_len;

	    pxor_r2r(mm0, mm0);
	    MOV_P2R(PIX_WHITE, mm7, mm0)
	    MOV_P2R(mul_col, mm5, mm0)
	    while (dst < dst_end)
	      {
		MOV_P2R(*src, mm1, mm0)
		MUL4_SYM_R2R(mm5, mm1, mm7)
		MOV_R2P(mm1, *dst, mm0)
		src++;  dst++;
	      }
	  }
	return;
     }

   if (src_len < dst_len)
     {
	DATA32  dsxx = ((src_len << 16) / dst_len);
	DATA32  sxx = 0;
	int     sx = sxx >> 16;
	DATA32  *dst_end = dst + dst_len;

	pxor_r2r(mm0, mm0);
	if (mul)
	  {
	    MOV_P2R(PIX_WHITE, mm7, mm0)
	    MOV_P2R(mul_col, mm5, mm0)
	  }
	while (dst < dst_end)
	  {
	    DATA32   a, p2, p1 = 0;

	    sx = (sxx >> 16);
	    if (sx < src_len)
		p1 = *(src + sx);
	    p2 = p1;
	    if ((sx + 1) < src_len)
	        p2 = *(src + sx + 1);
	    a = 256 - ((sxx - (sx << 16)) >> 8);
	    MOV_A2R(a, mm3)
	    MOV_P2R(p1, mm1, mm0)
	    MOV_P2R(p2, mm2, mm0)
	    INTERP_256_R2R(mm3, mm2, mm1)
	    if (mul)
	      {
		MUL4_SYM_R2R(mm5, mm1, mm7)
	      }
	    MOV_R2P(mm1, *dst, mm0)
	    dst++;  sxx += dsxx;
	  }
     }
   else
     {
	DATA32  dtxx = ((dst_len << 16) / src_len);
	DATA32  da = dtxx >> 8;
	DATA32  txx = 0, ntxx = txx;
	int     tx = txx >> 16, ntx, x = 0;
	DATA32  *dst_end = dst + dst_len;
	DATA32  *src_end = src + src_len;

	pxor_r2r(mm0, mm0);
	pxor_r2r(mm2, mm2);
	MOV_A2R(da, mm6)
	if (mul)
	  {
	    MOV_P2R(PIX_WHITE, mm7, mm0)
	    MOV_P2R(mul_col, mm5, mm0)
	  }
	while (dst < dst_end)
	  {
	    DATA32  a;

	    pxor_r2r(mm1, mm1);
	    if (src < src_end)
	      {
		MOV_P2R(*src, mm1, mm0)
	      }
	    ntxx += dtxx;
	    ntx = ntxx >> 16;
	    if (ntx != tx)
	      {
		if (ntx > x)
		  {
		    a = 256 - ((txx - (tx << 16)) >> 8);
		    MOV_A2R(a, mm3)
		    MUL4_256_R2R(mm1, mm3)
		    paddw_r2r(mm3, mm2);
		    if (mul)
		      {
			MUL4_SYM_R2R(mm5, mm2, mm7)
		      }
		    MOV_R2P(mm2, *dst, mm0)
		    dst++;
		    x++;
		  }
		a = 1 + ((ntxx - (ntx << 16)) >> 8);
		MOV_A2R(a, mm2)
		MUL4_256_R2R(mm1, mm2)
		tx = ntx;
	      }
	    else if (tx == x)
	      {
		MUL4_256_R2R(mm6, mm1)
		paddw_r2r(mm1, mm2);
	      }
	    src++;
	    txx = ntxx;
	  }
     }
}
#endif

void
evas_common_scale_rgba_span(DATA32 *src, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len)
{
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	evas_common_scale_rgba_span_mmx(src, src_len, mul_col, dst, dst_len);
	return;
     }
#endif
#ifdef BUILD_C
   evas_common_scale_rgba_span_c(src, src_len, mul_col, dst, dst_len);
#endif
}

