#include "evas_common_soft16.h"
#include "evas_soft16_scanline_blend.c"

static void
_soft16_image_draw_unscaled_solid_solid(Soft16_Image *src, Soft16_Image *dst,
					RGBA_Draw_Context *dc __UNUSED__,
					int src_offset, int dst_offset,
					int w, int h)
{
   DATA16 *src_itr, *dst_itr;
   int y;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_solid_solid(src_itr, dst_itr, w);
	src_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static void
_soft16_image_draw_unscaled_transp_solid(Soft16_Image *src, Soft16_Image *dst,
					 RGBA_Draw_Context *dc __UNUSED__,
					 int src_offset, int dst_offset,
					 int w, int h)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_transp_solid(src_itr, alpha_itr, dst_itr, w);
	src_itr += src->stride;
	alpha_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static inline void
_soft16_image_draw_unscaled_no_mul(Soft16_Image *src, Soft16_Image *dst,
                                   RGBA_Draw_Context *dc,
                                   int src_offset, int dst_offset,
                                   int width, int height)
{
   if (src->cache_entry.flags.alpha && (!dst->cache_entry.flags.alpha))
      _soft16_image_draw_unscaled_transp_solid(src, dst, dc,
                                               src_offset, dst_offset,
                                               width, height);
   else if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha))
      _soft16_image_draw_unscaled_solid_solid(src, dst, dc,
                                              src_offset, dst_offset,
                                              width, height);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->cache_entry.flags.alpha=%d, "
              "dst->cache_entry.flags.alpha=%d, WITHOUT COLOR MUL\n",
              src->cache_entry.flags.alpha, dst->cache_entry.flags.alpha);
}

static void
_soft16_image_draw_unscaled_solid_solid_mul_alpha(Soft16_Image *src,
                                                  Soft16_Image *dst,
                                                  RGBA_Draw_Context *dc __UNUSED__,
                                                  int src_offset,
                                                  int dst_offset,
                                                  int w, int h, DATA8 a)
{
   DATA16 *src_itr, *dst_itr;
   int y;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_solid_solid_mul_alpha(src_itr, dst_itr, w, a);
	src_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static void
_soft16_image_draw_unscaled_transp_solid_mul_alpha(Soft16_Image *src,
                                                   Soft16_Image *dst,
                                                   RGBA_Draw_Context *dc __UNUSED__,
                                                   int src_offset,
                                                   int dst_offset,
                                                   int w, int h, DATA8 a)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_transp_solid_mul_alpha(src_itr, alpha_itr,
                                                      dst_itr, w, a);
	src_itr += src->stride;
	alpha_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static inline void
_soft16_image_draw_unscaled_mul_alpha(Soft16_Image *src, Soft16_Image *dst,
                                      RGBA_Draw_Context *dc,
                                      int src_offset, int dst_offset,
                                      int width, int height, DATA8 a)
{
   if (src->cache_entry.flags.alpha && (!dst->cache_entry.flags.alpha))
      _soft16_image_draw_unscaled_transp_solid_mul_alpha
         (src, dst, dc, src_offset, dst_offset, width, height, a);
   else if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha))
      _soft16_image_draw_unscaled_solid_solid_mul_alpha
         (src, dst, dc, src_offset, dst_offset, width, height, a);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->cache_entry.flags.alpha=%d, "
              "dst->cache_entry.flags.alpha=%d, WITH ALPHA MUL %d\n",
              src->cache_entry.flags.alpha, dst->cache_entry.flags.alpha, A_VAL(&dc->mul.col));
}

static void
_soft16_image_draw_unscaled_solid_solid_mul_color(Soft16_Image *src,
                                                  Soft16_Image *dst,
                                                  RGBA_Draw_Context *dc __UNUSED__,
                                                  int src_offset,
                                                  int dst_offset,
                                                  int w, int h, DATA8 r,
						  DATA8 g, DATA8 b, DATA8 a)
{
   DATA16 *src_itr, *dst_itr;
   int y;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   if (a == 31)
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_solid_solid_mul_color_solid
               (src_itr, dst_itr, w, r, g, b);
            src_itr += src->stride;
            dst_itr += dst->stride;
         }
   else
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_solid_solid_mul_color_transp
               (src_itr, dst_itr, w, a, r, g, b);
            src_itr += src->stride;
            dst_itr += dst->stride;
         }
}

static void
_soft16_image_draw_unscaled_transp_solid_mul_color(Soft16_Image *src,
                                                   Soft16_Image *dst,
                                                   RGBA_Draw_Context *dc __UNUSED__,
                                                   int src_offset,
                                                   int dst_offset,
                                                   int w, int h, DATA8 r,
						   DATA8 g, DATA8 b, DATA8 a)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   if (a == 31)
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_transp_solid_mul_color_solid
               (src_itr, alpha_itr, dst_itr, w, r, g, b);
            src_itr += src->stride;
            alpha_itr += src->stride;
            dst_itr += dst->stride;
         }
   else
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_transp_solid_mul_color_transp
               (src_itr, alpha_itr, dst_itr, w, a, r, g, b);
            src_itr += src->stride;
            alpha_itr += src->stride;
            dst_itr += dst->stride;
         }
}

static inline void
_soft16_image_draw_unscaled_mul_color(Soft16_Image *src, Soft16_Image *dst,
                                      RGBA_Draw_Context *dc,
                                      int src_offset, int dst_offset,
                                      int width, int height,
				      DATA8 r, DATA8 g, DATA8 b, DATA8 a)
{
   if (src->cache_entry.flags.alpha && (!dst->cache_entry.flags.alpha))
     _soft16_image_draw_unscaled_transp_solid_mul_color
	(src, dst, dc, src_offset, dst_offset, width, height, r, g, b, a);
   else if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha))
     _soft16_image_draw_unscaled_solid_solid_mul_color
       (src, dst, dc, src_offset, dst_offset, width, height, r, g, b, a);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->cache_entry.flags.alpha=%d, "
              "dst->cache_entry.flags.alpha=%d, WITH COLOR MUL 0x%08x\n",
              src->cache_entry.flags.alpha, dst->cache_entry.flags.alpha, dc->mul.col);
}

static inline void
_soft16_image_draw_unscaled_mul(Soft16_Image *src, Soft16_Image *dst,
                                RGBA_Draw_Context *dc,
                                int src_offset, int dst_offset,
                                int width, int height, DATA8 r, DATA8 g,
				DATA8 b, DATA8 a)
{
   if ((a == r) && (a == (g >> 1)) && (a == b))
      _soft16_image_draw_unscaled_mul_alpha(src, dst, dc, src_offset,
                                            dst_offset, width, height, a);
   else
      _soft16_image_draw_unscaled_mul_color(src, dst, dc, src_offset,
                                            dst_offset, width, height,
					    r, g, b, a);
}

void
soft16_image_draw_unscaled(Soft16_Image *src, Soft16_Image *dst,
			   RGBA_Draw_Context *dc,
			   const Evas_Rectangle sr,
			   const Evas_Rectangle dr,
			   const Evas_Rectangle cr)
{
   int src_offset_rows, src_offset, dst_offset;
   DATA16 mul_rgb565;
   DATA8 r, g, b, a;

   if (!dc->mul.use)
     {
	r = b = a = 31;
	g = 63;
	mul_rgb565 = 0xffff;
     }
   else
     {
	a = A_VAL(&dc->mul.col) >> 3;
	if (a == 0)
	  return;

	r = R_VAL(&dc->mul.col) >> 3;
	g = G_VAL(&dc->mul.col) >> 2;
	b = B_VAL(&dc->mul.col) >> 3;

	if (r > a) r = a;
	if (g > (a << 1)) g = (a << 1);
	if (b > a) b = a;

	mul_rgb565 = (r << 11) || (g << 5) | b;
     }


   src_offset_rows = (cr.y - dr.y) + sr.y;
   src_offset = (src_offset_rows * src->stride) + (cr.x - dr.x) + sr.x;

   dst_offset = cr.x + (cr.y * dst->stride);

   if (mul_rgb565 == 0xffff)
       _soft16_image_draw_unscaled_no_mul(src, dst, dc, src_offset, dst_offset,
                                          cr.w, cr.h);
   else
       _soft16_image_draw_unscaled_mul(src, dst, dc, src_offset, dst_offset,
                                       cr.w, cr.h, r, g, b, a);
}
