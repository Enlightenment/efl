#include "evas_soft16.h"
#include "evas_soft16_scanline_fill.c"

static inline int
_is_empty_rectangle(const Evas_Rectangle *r)
{
   return (r->w < 1) || (r->h < 1);
}

static inline void
_soft16_rectangle_draw_solid_solid(Soft16_Image *dst, RGBA_Draw_Context *dc,
                                   int offset, int w, int h)
{
   DATA16 *dst_itr, rgb565;
   int i;

   dst_itr = dst->pixels + offset;
   rgb565 = RGB_565_FROM_COMPONENTS(R_VAL(&dc->col.col),
                                    G_VAL(&dc->col.col),
                                    B_VAL(&dc->col.col));

   for (i = 0; i < h; i++, dst_itr += dst->stride)
      _soft16_scanline_fill_solid_solid(dst_itr, w, rgb565);
}

static inline void
_soft16_rectangle_draw_transp_solid(Soft16_Image *dst, RGBA_Draw_Context *dc,
                                    int offset, int w, int h)
{
   char alpha;

   alpha = A_VAL(&dc->col.col) >> 3;
   if (alpha == 31) _soft16_rectangle_draw_solid_solid(dst, dc, offset, w, h);
   else if (alpha != 0)
      {
         DATA16 *dst_itr;
         DATA32 rgb565;
         int i;

         dst_itr = dst->pixels + offset;
         rgb565 = RGB_565_FROM_COMPONENTS(R_VAL(&dc->col.col),
                                          G_VAL(&dc->col.col),
                                          B_VAL(&dc->col.col));
         rgb565 = RGB_565_UNPACK(rgb565);

         for (i = 0; i < h; i++, dst_itr += dst->stride)
            _soft16_scanline_fill_transp_solid(dst_itr, w, rgb565, alpha);
      }
}

static void
_soft16_rectangle_draw_int(Soft16_Image *dst, RGBA_Draw_Context *dc,
                           Evas_Rectangle dr)
{
   int dst_offset;

   if (_is_empty_rectangle(&dr)) return;
   RECTS_CLIP_TO_RECT(dr.x, dr.y, dr.w, dr.h, 0, 0, dst->w, dst->h);
   if (_is_empty_rectangle(&dr)) return;

   if (dc->clip.use)
      RECTS_CLIP_TO_RECT(dr.x, dr.y, dr.w, dr.h, dc->clip.x,
                         dc->clip.y, dc->clip.w, dc->clip.h);
   if (_is_empty_rectangle(&dr)) return;
   if (A_VAL(&dc->col.col) == 0) return;

   dst_offset = dr.x + (dr.y * dst->w);

   if (!dst->have_alpha)
      {
         if (A_VAL(&dc->col.col) == 255)
            _soft16_rectangle_draw_solid_solid
               (dst, dc, dst_offset, dr.w, dr.h);
         else
            _soft16_rectangle_draw_transp_solid
               (dst, dc, dst_offset, dr.w, dr.h);
      }
   else
      fprintf(stderr,
              "Unsupported feature: drawing rectangle to non-opaque "
              "destination.\n");
}

void
soft16_rectangle_draw(Soft16_Image *dst, RGBA_Draw_Context *dc,
                      int x, int y, int w, int h)
{
   Evas_Rectangle dr;
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int c, cx, cy, cw, ch;
   int i;

   /* handle cutouts here! */
   dr.x = x;
   dr.y = y;
   dr.w = w;
   dr.h = h;

   if (_is_empty_rectangle(&dr)) return;
   if (!(RECTS_INTERSECT(dr.x, dr.y, dr.w, dr.h, 0, 0, dst->w, dst->h)))
     return;

   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
        _soft16_rectangle_draw_int(dst, dc, dr);
	return;
     }

   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->w, dst->h);
   evas_common_draw_context_clip_clip(dc, x, y, w, h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
	return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (i = 0; i < rects->active; ++i)
     {
	r = rects->rects + i;
	evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
        _soft16_rectangle_draw_int(dst, dc, dr);
     }
   evas_common_draw_context_apply_clear_cutouts(rects);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}

