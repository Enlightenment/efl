#include "evas_common.h"
#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

static DATA32 **scale_calc_y_points(DATA32 *src, int sw, int sh, int dh);
static int     *scale_calc_x_points(int sw, int dw);
static int     *scale_calc_a_points(int s, int d);
    
static DATA32 **
scale_calc_y_points(DATA32 *src, int sw, int sh, int dh)
{
   DATA32 **p;
   int i, val, inc;
   
   p = malloc((dh + 1) * sizeof(DATA32 *));
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
void
evas_common_scale_rgba_gen_mipmap_table(RGBA_Image *src)
{
   int w, h, i;
   
   w = src->image->w;
   h = src->image->h;
   do
     {
	w >>= 1;
	h >>= 1;
	if (w < 1) w = 1;
	if (h < 1) h = 1;
	src->mipmaps.num++;
     }
   while ((w > 1) && (h > 1));
   if (src->mipmaps.num <= 0) return;
   src->mipmaps.levels = malloc(sizeof(RGBA_Surface) * src->mipmaps.num);
   w = src->image->w;
   h = src->image->h;
   for (i = 0; i < src->mipmaps.num; i++)
     {
        w >>= 1;
	h >>= 1;
	if (w < 1) w = 1;
	if (h < 1) h = 1;	
	src->mipmaps.levels[i] = evas_common_image_surface_new();
	src->mipmaps.levels[i]->w = w;
	src->mipmaps.levels[i]->h = h;
     }
}
#endif

#ifdef BUILD_SCALE_SMOOTH
void
evas_common_scale_rgba_gen_mipmap_level(RGBA_Image *src, int l)
{
   RGBA_Surface *s_srf, *d_srf;
#ifdef BUILD_MMX
   int mmx, sse, sse2;
#endif   
   
   if (src->mipmaps.levels[l]->data) return;
   evas_common_image_surface_alloc(src->mipmaps.levels[l]);
   if (l > 0)
     evas_common_scale_rgba_gen_mipmap_level(src, l - 1);
   if (l > 0)
     s_srf = src->mipmaps.levels[l - 1];
   else
     s_srf = src->image;
   d_srf = src->mipmaps.levels[l];
#ifdef BUILD_MMX
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
   if (mmx)
     {
	if ((s_srf->w > 1) && (s_srf->h > 1))
	  evas_common_scale_rgba_mipmap_down_2x2_mmx(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	else if ((s_srf->w > 1) && (s_srf->h <= 1))
	  evas_common_scale_rgba_mipmap_down_2x1_mmx(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	else if ((s_srf->w <= 1) && (s_srf->h > 1))
	  evas_common_scale_rgba_mipmap_down_1x2_mmx(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
     }
   else
     {
#endif   
#ifdef BUILD_C
	if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     if ((s_srf->w > 1) && (s_srf->h > 1))
	       evas_common_scale_rgba_mipmap_down_2x2_c(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	     else if ((s_srf->w > 1) && (s_srf->h <= 1))
	       evas_common_scale_rgba_mipmap_down_2x1_c(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	     else if ((s_srf->w <= 1) && (s_srf->h > 1))
	       evas_common_scale_rgba_mipmap_down_1x2_c(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	  }
	else
	  {
	     if ((s_srf->w > 1) && (s_srf->h > 1))
	       evas_common_scale_rgb_mipmap_down_2x2_c(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	     else if ((s_srf->w > 1) && (s_srf->h <= 1))
	       evas_common_scale_rgb_mipmap_down_2x1_c(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	     else if ((s_srf->w <= 1) && (s_srf->h > 1))
	       evas_common_scale_rgb_mipmap_down_1x2_c(s_srf->data, d_srf->data, s_srf->w, s_srf->h);
	  }
#endif	
#ifdef BUILD_MMX
     }
#endif
}
#endif

#ifdef BUILD_SCALE_SMOOTH
#ifdef BUILD_SCALE_TRILINEAR
int
evas_common_scale_rgba_get_mipmap_surfaces(RGBA_Image *src, int src_region_w, int src_region_h, int dst_region_w, int dst_region_h, RGBA_Surface **srf1, RGBA_Surface **srf2)
{
   int scale_x, scale_y;
   int l, l1, l2;
   int mix, pw, ph;
   int w, h;
   int dw, dh;
   
   scale_x = (dst_region_w << 16) / src_region_w;
   scale_y = (dst_region_h << 16) / src_region_h;

   mix = 0;
   l1 = -1;
   l2 = -1;
   w = src->image->w;
   h = src->image->h;
   dw = (dst_region_w * src->image->w) / src_region_w;
   dh = (dst_region_h * src->image->h) / src_region_h;
   
   /* if we prefer smoothened use smaller scaling axis */
   if (scale_x < scale_y)
     /* use x axis */
     {
	for (l = 0; l < src->mipmaps.num; l++)
	  {
	     pw = w;
	     w >>=1;
	     if (dw > w)
	       {
		  mix = ((dw - w) << 8) / (pw - w);
		  l1 = l - 1;
		  l2 = l;
		  break;
	       }
	  }
     }
   else
     /* use y axis */
     {
	for (l = 0; l < src->mipmaps.num; l++)
	  {
	     ph = h;
	     h >>=1;
	     if (dh > h)
	       {
		  mix = ((dh - h) << 8) / (ph - h);
		  l1 = l - 1;
		  l2 = l;
		  break;
	       }
	  }
     }
   if (l1 == -1) *srf1 = src->image;
   else
     {
	evas_common_scale_rgba_gen_mipmap_level(src, l1);
	*srf1 = src->mipmaps.levels[l1];
     }
   if (l2 == -1) *srf2 = src->image;
   else
     {
	evas_common_scale_rgba_gen_mipmap_level(src, l2);
	*srf2 = src->mipmaps.levels[l2];
     }
   return mix;
}
#else
RGBA_Surface *
evas_common_scale_rgba_get_mipmap_surface(RGBA_Image *src, int src_region_w, int src_region_h, int dst_region_w, int dst_region_h)
{
   int scale_x, scale_y;
   int l;
   int pw, ph;
   int w, h;
   int dw, dh;
   RGBA_Surface *srf;
   
   scale_x = (dst_region_w << 16) / src_region_w;
   scale_y = (dst_region_h << 16) / src_region_h;

   w = src->image->w;
   h = src->image->h;
   dw = (dst_region_w * src->image->w) / src_region_w;
   dh = (dst_region_h * src->image->h) / src_region_h;
   
   /* if we prefer smoothened use larger scaling axis */
   if (scale_x > scale_y)
     /* use x axis */
     {
	for (l = 0; l < src->mipmaps.num; l++)
	  {
	     pw = w;
	     w >>=1;
	     if (dw > w) break;
	  }
     }
   else
     /* use y axis */
     {
	for (l = 0; l < src->mipmaps.num; l++)
	  {
	     ph = h;
	     h >>=1;
	     if (dh > h) break;
	  }
     }
   l--;
   srf = src->image;
   if (l == -1) srf = src->image;
   else
     {
	evas_common_scale_rgba_gen_mipmap_level(src, l);
	srf = src->mipmaps.levels[l];
     }
   return srf;
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
#  ifndef BUILD_SCALE_TRILINEAR   
   int mmx, sse, sse2;
#  endif
# endif   
   int c, cx, cy, cw, ch;
   Cutout_Rect *rects, *r;
   Evas_Object_List *l;
   /* handle cutouts here! */
   
   if ((dst_region_w <= 0) || (dst_region_h <= 0)) return;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->image->w, dst->image->h)))
     return;
# ifdef BUILD_MMX
#  ifndef BUILD_SCALE_TRILINEAR   
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
#  endif
# endif   
   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
# ifdef BUILD_MMX
#  ifndef BUILD_SCALE_TRILINEAR   
	if (mmx)
	  evas_common_scale_rgba_in_to_out_clip_smooth_mmx(src, dst, dc, 
					       src_region_x, src_region_y, 
					       src_region_w, src_region_h,
					       dst_region_x, dst_region_y,
					       dst_region_w, dst_region_h);
	else
#  endif	
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
#  ifndef BUILD_SCALE_TRILINEAR   
	if (mmx)
	  evas_common_scale_rgba_in_to_out_clip_smooth_mmx(src, dst, dc, 
					       src_region_x, src_region_y, 
					       src_region_w, src_region_h,
					       dst_region_x, dst_region_y,
					       dst_region_w, dst_region_h);
	else
#  endif	
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
