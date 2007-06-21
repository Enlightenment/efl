#include "evas_soft16.h"
#include "evas_soft16_scanline_blend.c"

static void
_soft16_image_draw_unscaled_solid_solid(Soft16_Image *src, Soft16_Image *dst,
					RGBA_Draw_Context *dc,
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
					 RGBA_Draw_Context *dc,
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
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_unscaled_transp_solid(src, dst, dc,
                                               src_offset, dst_offset,
                                               width, height);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_unscaled_solid_solid(src, dst, dc,
                                              src_offset, dst_offset,
                                              width, height);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITHOUT COLOR MUL\n",
              src->have_alpha, dst->have_alpha);
}

static void
_soft16_image_draw_unscaled_solid_solid_mul_alpha(Soft16_Image *src,
                                                  Soft16_Image *dst,
                                                  RGBA_Draw_Context *dc,
                                                  int src_offset,
                                                  int dst_offset,
                                                  int w, int h)
{
   DATA16 *src_itr, *dst_itr;
   int y, rel_alpha;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_solid_solid_mul_alpha(src_itr, dst_itr, w,
                                                     rel_alpha);
	src_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static void
_soft16_image_draw_unscaled_transp_solid_mul_alpha(Soft16_Image *src,
                                                   Soft16_Image *dst,
                                                   RGBA_Draw_Context *dc,
                                                   int src_offset,
                                                   int dst_offset,
                                                   int w, int h)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y, rel_alpha;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_transp_solid_mul_alpha(src_itr, alpha_itr,
                                                      dst_itr, w, rel_alpha);
	src_itr += src->stride;
	alpha_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static inline void
_soft16_image_draw_unscaled_mul_alpha(Soft16_Image *src, Soft16_Image *dst,
                                      RGBA_Draw_Context *dc,
                                      int src_offset, int dst_offset,
                                      int width, int height)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_unscaled_transp_solid_mul_alpha
         (src, dst, dc, src_offset, dst_offset, width, height);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_unscaled_solid_solid_mul_alpha
         (src, dst, dc, src_offset, dst_offset, width, height);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITH ALPHA MUL %d\n",
              src->have_alpha, dst->have_alpha, A_VAL(&dc->mul.col));
}

static void
_soft16_image_draw_unscaled_solid_solid_mul_color(Soft16_Image *src,
                                                  Soft16_Image *dst,
                                                  RGBA_Draw_Context *dc,
                                                  int src_offset,
                                                  int dst_offset,
                                                  int w, int h)
{
   DATA16 *src_itr, *dst_itr;
   int y, rel_alpha, r, g, b;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;

   r = R_VAL(&dc->mul.col);
   g = G_VAL(&dc->mul.col);
   b = B_VAL(&dc->mul.col);
   /* we'll divide by 256 to make it faster, try to improve things a bit */
   if (r > 127) r++;
   if (g > 127) g++;
   if (b > 127) b++;

   if (rel_alpha == 31)
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
               (src_itr, dst_itr, w, rel_alpha, r, g, b);
            src_itr += src->stride;
            dst_itr += dst->stride;
         }
}

static void
_soft16_image_draw_unscaled_transp_solid_mul_color(Soft16_Image *src,
                                                   Soft16_Image *dst,
                                                   RGBA_Draw_Context *dc,
                                                   int src_offset,
                                                   int dst_offset,
                                                   int w, int h)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y, rel_alpha, r, g, b;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   r = R_VAL(&dc->mul.col);
   g = G_VAL(&dc->mul.col);
   b = B_VAL(&dc->mul.col);
   /* we'll divide by 256 to make it faster, try to improve things a bit */
   if (r > 127) r++;
   if (g > 127) g++;
   if (b > 127) b++;

   if (rel_alpha == 0)
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
               (src_itr, alpha_itr, dst_itr, w, rel_alpha, r, g, b);
            src_itr += src->stride;
            alpha_itr += src->stride;
            dst_itr += dst->stride;
         }
}

static inline void
_soft16_image_draw_unscaled_mul_color(Soft16_Image *src, Soft16_Image *dst,
                                      RGBA_Draw_Context *dc,
                                      int src_offset, int dst_offset,
                                      int width, int height)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_unscaled_transp_solid_mul_color
         (src, dst, dc, src_offset, dst_offset, width, height);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_unscaled_solid_solid_mul_color
         (src, dst, dc, src_offset, dst_offset, width, height);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITH COLOR MUL 0x%08x\n",
              src->have_alpha, dst->have_alpha, dc->mul.col);
}

static inline void
_soft16_image_draw_unscaled_mul(Soft16_Image *src, Soft16_Image *dst,
                                RGBA_Draw_Context *dc,
                                int src_offset, int dst_offset,
                                int width, int height)
{
   if ((A_VAL(&dc->mul.col) == R_VAL(&dc->mul.col)) &&
       (A_VAL(&dc->mul.col) == G_VAL(&dc->mul.col)) &&
       (A_VAL(&dc->mul.col) == B_VAL(&dc->mul.col)))
      _soft16_image_draw_unscaled_mul_alpha(src, dst, dc, src_offset,
                                            dst_offset, width, height);
   else
      _soft16_image_draw_unscaled_mul_color(src, dst, dc, src_offset,
                                            dst_offset, width, height);
}

void
soft16_image_draw_unscaled(Soft16_Image *src, Soft16_Image *dst,
			   RGBA_Draw_Context *dc,
			   const Evas_Rectangle sr,
			   const Evas_Rectangle dr,
			   const Evas_Rectangle cr)
{
   int src_offset_rows, src_offset, dst_offset;

   src_offset_rows = (cr.y - dr.y) + sr.y;
   src_offset = (src_offset_rows * src->stride) + (cr.x - dr.x) + sr.x;

   dst_offset = cr.x + (cr.y * dst->stride);

   if ((!dc->mul.use) || (dc->mul.col == 0xffffffff))
       _soft16_image_draw_unscaled_no_mul(src, dst, dc, src_offset, dst_offset,
                                          cr.w, cr.h);
   else if (dc->mul.col != 0x00000000)
       _soft16_image_draw_unscaled_mul(src, dst, dc, src_offset, dst_offset,
                                       cr.w, cr.h);
}
