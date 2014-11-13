#include "evas_common_private.h"
#include "evas_scale_smooth.h"
#include "evas_blend_private.h"
#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

#define SCALE_CALC_X_POINTS(P, SW, DW, CX, CW) \
  P = alloca((CW + 1) * sizeof (int));         \
  scale_calc_x_points(P, SW, DW, CX, CW);

#define SCALE_CALC_Y_POINTS(P, SRC, SW, SH, DH, CY, CH) \
  P = alloca((CH + 1) * sizeof (DATA32 *));             \
  scale_calc_y_points(P, SRC, SW, SH, DH, CY, CH);

#define SCALE_CALC_A_POINTS(P, S, D, C, CC) \
  P = alloca(CC * sizeof (int));            \
  scale_calc_a_points(P, S, D, C, CC);

static void scale_calc_y_points(DATA32 **p, DATA32 *src, int sw, int sh, int dh, int cy, int ch);
static void scale_calc_x_points(int *p, int sw, int dw, int cx, int cw);
static void scale_calc_a_points(int *p, int s, int d, int c, int cc);

static void
scale_calc_y_points(DATA32** p, DATA32 *src, int sw, int sh, int dh, int cy, int ch)
{
   int i, val, inc;
   if (sh > SCALE_SIZE_MAX) return;
   val = 0;
   inc = (sh << 16) / dh;
   for (i = 0; i < dh; i++)
     {
        if ((i >= cy) && (i < (cy + ch)))
           p[i - cy] = src + ((val >> 16) * sw);
	val += inc;
     }
   if ((i >= cy) && (i < (cy + ch)))
      p[i - cy] = p[i - cy - 1];
}

static void
scale_calc_x_points(int *p, int sw, int dw, int cx, int cw)
{
   int i, val, inc;
   if (sw > SCALE_SIZE_MAX) return;
   val = 0;
   inc = (sw << 16) / dw;
   for (i = 0; i < dw; i++)
     {
        if ((i >= cx) && (i < (cx + cw)))
           p[i - cx] = val >> 16;
	val += inc;
     }
   if ((i >= cx) && (i < (cx + cw)))
      p[i - cx] = p[i - cx - 1];
}

static void
scale_calc_a_points(int *p, int s, int d, int c, int cc)
{
   int i, val, inc;

   if (s > SCALE_SIZE_MAX) return;
   if (d >= s)
     {
	val = 0;
	inc = (s << 16) / d;
	for (i = 0; i < d; i++)
	  {
             if ((i >= c) && (i < (c + cc)))
               {
                  p[i - c] = (val >> 8) - ((val >> 8) & 0xffffff00);
                  if ((val >> 16) >= (s - 1)) p[i - c] = 0;
               }
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
             if ((i >= c) && (i < (c + cc)))
                p[i - c] = ap | (Cp << 16);
	     val += inc;
	  }
     }
}

#ifdef BUILD_MMX
# undef SCALE_FUNC
# define SCALE_FUNC _evas_common_scale_rgba_in_to_out_clip_smooth_mmx
# undef SCALE_USING_MMX
# define SCALE_USING_MMX
# include "evas_scale_smooth_scaler.c"
#endif

#ifdef BUILD_NEON
# undef SCALE_FUNC
# undef SCALE_USING_NEON
# define SCALE_USING_NEON
# define SCALE_FUNC _evas_common_scale_rgba_in_to_out_clip_smooth_neon
# include "evas_scale_smooth_scaler.c"
# undef SCALE_USING_NEON
#endif

#undef SCALE_FUNC
#define SCALE_FUNC _evas_common_scale_rgba_in_to_out_clip_smooth_c
#undef SCALE_USING_MMX
#include "evas_scale_smooth_scaler.c"

#ifdef BUILD_MMX
Eina_Bool
evas_common_scale_rgba_in_to_out_clip_smooth_mmx(RGBA_Image *src, RGBA_Image *dst,
                                                 RGBA_Draw_Context *dc,
                                                 int src_region_x, int src_region_y,
                                                 int src_region_w, int src_region_h,
                                                 int dst_region_x, int dst_region_y,
                                                 int dst_region_w, int dst_region_h)
{
   int clip_x, clip_y, clip_w, clip_h;
   DATA32 mul_col;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = 0;
	clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   _evas_common_scale_rgba_in_to_out_clip_smooth_mmx
     (src, dst,
      clip_x, clip_y, clip_w, clip_h,
      mul_col, dc->render_op,
      src_region_x, src_region_y, src_region_w, src_region_h,
      dst_region_x, dst_region_y, dst_region_w, dst_region_h,
      dc->clip.mask, dc->clip.mask_x, dc->clip.mask_y);

   return EINA_TRUE;
}
#endif

#ifdef BUILD_NEON
Eina_Bool
evas_common_scale_rgba_in_to_out_clip_smooth_neon(RGBA_Image *src, RGBA_Image *dst,
                                                 RGBA_Draw_Context *dc,
                                                 int src_region_x, int src_region_y,
                                                 int src_region_w, int src_region_h,
                                                 int dst_region_x, int dst_region_y,
                                                 int dst_region_w, int dst_region_h)
{
   int clip_x, clip_y, clip_w, clip_h;
   DATA32 mul_col;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = 0;
	clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   _evas_common_scale_rgba_in_to_out_clip_smooth_neon
     (src, dst,
      clip_x, clip_y, clip_w, clip_h,
      mul_col, dc->render_op,
      src_region_x, src_region_y, src_region_w, src_region_h,
      dst_region_x, dst_region_y, dst_region_w, dst_region_h,
      dc->clip.mask, dc->clip.mask_x, dc->clip.mask_y);

   return EINA_TRUE;
}
#endif

Eina_Bool
evas_common_scale_rgba_in_to_out_clip_smooth_c(RGBA_Image *src, RGBA_Image *dst,
                                               RGBA_Draw_Context *dc,
                                               int src_region_x, int src_region_y,
                                               int src_region_w, int src_region_h,
                                               int dst_region_x, int dst_region_y,
                                               int dst_region_w, int dst_region_h)
{
   int clip_x, clip_y, clip_w, clip_h;
   DATA32 mul_col;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = 0;
	clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   _evas_common_scale_rgba_in_to_out_clip_smooth_c
     (src, dst,
      clip_x, clip_y, clip_w, clip_h,
      mul_col, dc->render_op,
      src_region_x, src_region_y, src_region_w, src_region_h,
      dst_region_x, dst_region_y, dst_region_w, dst_region_h,
      dc->clip.mask, dc->clip.mask_x, dc->clip.mask_y);

   return EINA_TRUE;
}

EAPI Eina_Bool
evas_common_scale_rgba_in_to_out_clip_smooth(RGBA_Image *src, RGBA_Image *dst,
                                             RGBA_Draw_Context *dc,
                                             int src_region_x, int src_region_y,
                                             int src_region_w, int src_region_h,
                                             int dst_region_x, int dst_region_y,
                                             int dst_region_w, int dst_region_h)
{
   Evas_Common_Scale_In_To_Out_Clip_Cb cb;
#ifdef BUILD_MMX
   int mmx, sse, sse2;

   evas_common_cpu_can_do(&mmx, &sse, &sse2);
   if (mmx)
     cb = evas_common_scale_rgba_in_to_out_clip_smooth_mmx;
   else
#endif
#ifdef BUILD_NEON
     if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
       cb = evas_common_scale_rgba_in_to_out_clip_smooth_neon;
   else
#endif
     cb = evas_common_scale_rgba_in_to_out_clip_smooth_c;

   return evas_common_scale_rgba_in_to_out_clip_cb(src, dst, dc,
                                                   src_region_x, src_region_y,
                                                   src_region_w, src_region_h,
                                                   dst_region_x, dst_region_y,
                                                   dst_region_w, dst_region_h,
                                                   cb);
}

EAPI void
evas_common_scale_rgba_smooth_draw(RGBA_Image *src, RGBA_Image *dst, int dst_clip_x, int dst_clip_y, int dst_clip_w, int dst_clip_h, DATA32 mul_col, int render_op, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h, RGBA_Image *mask_ie, int mask_x, int mask_y)
{
#ifdef BUILD_MMX
   int mmx, sse, sse2;

   evas_common_cpu_can_do(&mmx, &sse, &sse2);
   if (mmx)
     _evas_common_scale_rgba_in_to_out_clip_smooth_mmx
       (src, dst,
        dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h,
        mul_col, render_op,
        src_region_x, src_region_y, src_region_w, src_region_h,
        dst_region_x, dst_region_y, dst_region_w, dst_region_h,
        mask_ie, mask_x, mask_y);
   else
#endif
#ifdef BUILD_NEON
     if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
       _evas_common_scale_rgba_in_to_out_clip_smooth_neon
     (src, dst,
         dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h,
         mul_col, render_op,
         src_region_x, src_region_y, src_region_w, src_region_h,
         dst_region_x, dst_region_y, dst_region_w, dst_region_h,
         mask_ie, mask_x, mask_y);
   else
#endif
     _evas_common_scale_rgba_in_to_out_clip_smooth_c
       (src, dst,
        dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h,
        mul_col, render_op,
        src_region_x, src_region_y, src_region_w, src_region_h,
        dst_region_x, dst_region_y, dst_region_w, dst_region_h,
        mask_ie, mask_x, mask_y);
}

EAPI void
evas_common_scale_rgba_in_to_out_clip_smooth_do(const Cutout_Rects *reuse,
						const Eina_Rectangle *clip,
						RGBA_Image *src, RGBA_Image *dst,
						RGBA_Draw_Context *dc,
						int src_region_x, int src_region_y,
						int src_region_w, int src_region_h,
						int dst_region_x, int dst_region_y,
						int dst_region_w, int dst_region_h)
{
# ifdef BUILD_MMX
   int mmx, sse, sse2;
# endif
   Eina_Rectangle area;
   Cutout_Rect *r;
   int i;

# ifdef BUILD_MMX
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
# endif
   if (!reuse)
     {
        evas_common_draw_context_clip_clip(dc, clip->x, clip->y, clip->w, clip->h);
# ifdef BUILD_MMX
	if (mmx)
	  evas_common_scale_rgba_in_to_out_clip_smooth_mmx(src, dst, dc,
					       src_region_x, src_region_y,
					       src_region_w, src_region_h,
					       dst_region_x, dst_region_y,
					       dst_region_w, dst_region_h);
	else
# endif
#ifdef BUILD_NEON
          if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
            evas_common_scale_rgba_in_to_out_clip_smooth_neon(src, dst, dc,
                                                              src_region_x, src_region_y,
                                                              src_region_w, src_region_h,
                                                              dst_region_x, dst_region_y,
                                                              dst_region_w, dst_region_h);
        else
#endif
	  evas_common_scale_rgba_in_to_out_clip_smooth_c(src, dst, dc,
                                                         src_region_x, src_region_y,
                                                         src_region_w, src_region_h,
                                                         dst_region_x, dst_region_y,
                                                         dst_region_w, dst_region_h);
        return;
     }

   for (i = 0; i < reuse->active; ++i)
     {
        r = reuse->rects + i;

        EINA_RECTANGLE_SET(&area, r->x, r->y, r->w, r->h);
        if (!eina_rectangle_intersection(&area, clip)) continue ;
        evas_common_draw_context_set_clip(dc, area.x, area.y, area.w, area.h);
# ifdef BUILD_MMX
	if (mmx)
	  evas_common_scale_rgba_in_to_out_clip_smooth_mmx(src, dst, dc,
					       src_region_x, src_region_y,
					       src_region_w, src_region_h,
					       dst_region_x, dst_region_y,
					       dst_region_w, dst_region_h);
	else
# endif
#ifdef BUILD_NEON
          if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
            evas_common_scale_rgba_in_to_out_clip_smooth_neon(src, dst, dc,
                                                              src_region_x, src_region_y,
                                                              src_region_w, src_region_h,
                                                              dst_region_x, dst_region_y,
                                                              dst_region_w, dst_region_h);
        else
#endif
            evas_common_scale_rgba_in_to_out_clip_smooth_c(src, dst, dc,
                                                         src_region_x, src_region_y,
                                                         src_region_w, src_region_h,
                                                         dst_region_x, dst_region_y,
                                                         dst_region_w, dst_region_h);
     }
}
