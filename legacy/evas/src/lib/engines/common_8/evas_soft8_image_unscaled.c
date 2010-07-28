#include "evas_common_soft8.h"
#include "evas_soft8_scanline_blend.c"

static void
_soft8_image_draw_unscaled_solid_solid(Soft8_Image * src, Soft8_Image * dst,
                                       RGBA_Draw_Context * dc __UNUSED__,
                                       int src_offset, int dst_offset,
                                       int w, int h)
{
   DATA8 *src_itr, *dst_itr;
   int y;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
        _soft8_scanline_blend_solid_solid(src_itr, dst_itr, w);
        src_itr += src->stride;
        dst_itr += dst->stride;
     }
}

static void
_soft8_image_draw_unscaled_transp_solid(Soft8_Image * src, Soft8_Image * dst,
                                        RGBA_Draw_Context * dc __UNUSED__,
                                        int src_offset, int dst_offset,
                                        int w, int h)
{
   DATA8 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
        _soft8_scanline_blend_transp_solid(src_itr, alpha_itr, dst_itr, w);
        src_itr += src->stride;
        alpha_itr += src->stride;
        dst_itr += dst->stride;
     }
}

static inline void
_soft8_image_draw_unscaled_no_mul(Soft8_Image * src, Soft8_Image * dst,
                                  RGBA_Draw_Context * dc,
                                  int src_offset, int dst_offset,
                                  int width, int height)
{
   if (src->cache_entry.flags.alpha && (!dst->cache_entry.flags.alpha))
      _soft8_image_draw_unscaled_transp_solid(src, dst, dc,
                                              src_offset, dst_offset,
                                              width, height);
   else if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha))
      _soft8_image_draw_unscaled_solid_solid(src, dst, dc,
                                             src_offset, dst_offset,
                                             width, height);
   else
      ERR("Unsupported draw of unscaled images src->cache_entry.flags.alpha=%d, "
          "dst->cache_entry.flags.alpha=%d, WITHOUT COLOR MUL",
          src->cache_entry.flags.alpha, dst->cache_entry.flags.alpha);
}

static void
_soft8_image_draw_unscaled_solid_solid_mul_alpha(Soft8_Image * src,
                                                 Soft8_Image * dst,
                                                 RGBA_Draw_Context *
                                                 dc __UNUSED__, int src_offset,
                                                 int dst_offset, int w, int h,
                                                 DATA8 a)
{
   DATA8 *src_itr, *dst_itr;
   int y;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
        _soft8_scanline_blend_solid_solid_mul_alpha(src_itr, dst_itr, w, a);
        src_itr += src->stride;
        dst_itr += dst->stride;
     }
}

static void
_soft8_image_draw_unscaled_transp_solid_mul_alpha(Soft8_Image * src,
                                                  Soft8_Image * dst,
                                                  RGBA_Draw_Context *
                                                  dc __UNUSED__, int src_offset,
                                                  int dst_offset, int w, int h,
                                                  DATA8 a)
{
   DATA8 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
        _soft8_scanline_blend_transp_solid_mul_alpha(src_itr, alpha_itr,
                                                     dst_itr, w, a);
        src_itr += src->stride;
        alpha_itr += src->stride;
        dst_itr += dst->stride;
     }
}

static inline void
_soft8_image_draw_unscaled_mul_alpha(Soft8_Image * src, Soft8_Image * dst,
                                     RGBA_Draw_Context * dc,
                                     int src_offset, int dst_offset,
                                     int width, int height, DATA8 a)
{
   if (src->cache_entry.flags.alpha && (!dst->cache_entry.flags.alpha))
      _soft8_image_draw_unscaled_transp_solid_mul_alpha
          (src, dst, dc, src_offset, dst_offset, width, height, a);
   else if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha))
      _soft8_image_draw_unscaled_solid_solid_mul_alpha
          (src, dst, dc, src_offset, dst_offset, width, height, a);
   else
      ERR("Unsupported draw of unscaled images src->cache_entry.flags.alpha=%d, "
          "dst->cache_entry.flags.alpha=%d, WITH ALPHA MUL %d",
          src->cache_entry.flags.alpha, dst->cache_entry.flags.alpha, A_VAL(&dc->mul.col));
}

static void
_soft8_image_draw_unscaled_solid_solid_mul_color(Soft8_Image * src,
                                                 Soft8_Image * dst,
                                                 RGBA_Draw_Context *
                                                 dc __UNUSED__, int src_offset,
                                                 int dst_offset, int w, int h,
                                                 DATA8 r, DATA8 g, DATA8 b,
                                                 DATA8 a)
{
   DATA8 *src_itr, *dst_itr;
   int y;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   if (a == 0xff)
      for (y = 0; y < h; y++)
        {
           _soft8_scanline_blend_solid_solid_mul_color_solid
               (src_itr, dst_itr, w, r, g, b);
           src_itr += src->stride;
           dst_itr += dst->stride;
        }
   else
      for (y = 0; y < h; y++)
        {
           _soft8_scanline_blend_solid_solid_mul_color_transp
               (src_itr, dst_itr, w, a, r, g, b);
           src_itr += src->stride;
           dst_itr += dst->stride;
        }
}

static void
_soft8_image_draw_unscaled_transp_solid_mul_color(Soft8_Image * src,
                                                  Soft8_Image * dst,
                                                  RGBA_Draw_Context *
                                                  dc __UNUSED__, int src_offset,
                                                  int dst_offset, int w, int h,
                                                  DATA8 r, DATA8 g, DATA8 b,
                                                  DATA8 a)
{
   DATA8 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   if (a == 0xff)
      for (y = 0; y < h; y++)
        {
           _soft8_scanline_blend_transp_solid_mul_color_solid
               (src_itr, alpha_itr, dst_itr, w, r, g, b);
           src_itr += src->stride;
           alpha_itr += src->stride;
           dst_itr += dst->stride;
        }
   else
      for (y = 0; y < h; y++)
        {
           _soft8_scanline_blend_transp_solid_mul_color_transp
               (src_itr, alpha_itr, dst_itr, w, a, r, g, b);
           src_itr += src->stride;
           alpha_itr += src->stride;
           dst_itr += dst->stride;
        }
}

static inline void
_soft8_image_draw_unscaled_mul_color(Soft8_Image * src, Soft8_Image * dst,
                                     RGBA_Draw_Context * dc,
                                     int src_offset, int dst_offset,
                                     int width, int height,
                                     DATA8 r, DATA8 g, DATA8 b, DATA8 a)
{
   if (src->cache_entry.flags.alpha && (!dst->cache_entry.flags.alpha))
      _soft8_image_draw_unscaled_transp_solid_mul_color
          (src, dst, dc, src_offset, dst_offset, width, height, r, g, b, a);
   else if ((!src->cache_entry.flags.alpha) && (!dst->cache_entry.flags.alpha))
      _soft8_image_draw_unscaled_solid_solid_mul_color
          (src, dst, dc, src_offset, dst_offset, width, height, r, g, b, a);
   else
      ERR("Unsupported draw of unscaled images src->cache_entry.flags.alpha=%d, "
          "dst->cache_entry.flags.alpha=%d, WITH COLOR MUL 0x%08x",
          src->cache_entry.flags.alpha, dst->cache_entry.flags.alpha, dc->mul.col);
}

static inline void
_soft8_image_draw_unscaled_mul(Soft8_Image * src, Soft8_Image * dst,
                               RGBA_Draw_Context * dc,
                               int src_offset, int dst_offset,
                               int width, int height, DATA8 r, DATA8 g,
                               DATA8 b, DATA8 a)
{
   if ((a == r) && (a == g) && (a == b))
      _soft8_image_draw_unscaled_mul_alpha(src, dst, dc, src_offset,
                                           dst_offset, width, height, a);
   else
      _soft8_image_draw_unscaled_mul_color(src, dst, dc, src_offset,
                                           dst_offset, width, height,
                                           r, g, b, a);
}

void
soft8_image_draw_unscaled(Soft8_Image * src, Soft8_Image * dst,
                          RGBA_Draw_Context * dc,
                          const Eina_Rectangle sr,
                          const Eina_Rectangle dr, const Eina_Rectangle cr)
{
   int src_offset_rows, src_offset, dst_offset;
   DATA8 mul_gry8;
   DATA8 r, g, b, a;

   if (!dc->mul.use)
     {
        r = g = b = a = 0xff;
        mul_gry8 = 0xff;
     }
   else
     {
        a = A_VAL(&dc->mul.col);
        if (a == 0)
           return;

        r = R_VAL(&dc->mul.col);
        g = G_VAL(&dc->mul.col);
        b = B_VAL(&dc->mul.col);

        if (r > a)
           r = a;
        if (g > a)
           g = a;
        if (b > a)
           b = a;

        mul_gry8 = GRY_8_FROM_COMPONENTS(r, g, b);
     }

   src_offset_rows = (cr.y - dr.y) + sr.y;
   src_offset = (src_offset_rows * src->stride) + (cr.x - dr.x) + sr.x;

   dst_offset = cr.x + (cr.y * dst->stride);

   if (mul_gry8 == 0xff)
      _soft8_image_draw_unscaled_no_mul(src, dst, dc, src_offset, dst_offset,
                                        cr.w, cr.h);
   else
      _soft8_image_draw_unscaled_mul(src, dst, dc, src_offset, dst_offset,
                                     cr.w, cr.h, r, g, b, a);
}
