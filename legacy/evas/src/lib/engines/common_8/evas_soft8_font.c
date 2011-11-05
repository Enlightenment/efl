#include "evas_common_soft8.h"
#include "evas_soft8_scanline_blend.c"

EFL_ALWAYS_INLINE void
_glyph_pt_mask_solid_solid(DATA8 * dst, const DATA8 gry8, const DATA8 * mask)
{
   DATA8 alpha = *mask;

   if (alpha == 0xff)
      *dst = gry8;
   else if (alpha > 0)
     {
        *dst = GRY_8_BLEND_UNMUL(gry8, *dst, alpha);
     }
}

static void
_glyph_scanline_mask_solid_solid(DATA8 * dst,
                                 int size, const DATA8 gry8, const DATA8 * mask)
{
   DATA8 *start, *end;

   start = dst;
   pld(start, 0);
   pld(mask, 0);
   end = start + (size & ~3);

   while (start < end)
     {
        pld(start, 8);
        pld(mask, 4);
        UNROLL4(
                  {
                  _glyph_pt_mask_solid_solid(start, gry8, mask);
                  start++; mask++;}
        );
     }

   end = start + (size & 3);
   for (; start < end; start++, mask++)
      _glyph_pt_mask_solid_solid(start, gry8, mask);
}

EFL_ALWAYS_INLINE void
_glyph_pt_mask_transp_solid(DATA8 * dst,
                            DATA8 gry8, DATA8 alpha, const DATA8 * mask)
{
   int rel_alpha;

   rel_alpha = *mask;
   alpha = (alpha * rel_alpha) >> 8;
   if (alpha == 0)
      return;

   alpha++;

   *dst = GRY_8_BLEND((gry8 * rel_alpha) >> 8, *dst, alpha);
}

static void
_glyph_scanline_mask_transp_solid(DATA8 * dst,
                                  int size,
                                  const DATA8 gry8,
                                  const DATA8 rel_alpha, const DATA8 * mask)
{
   DATA8 *start, *end;

   start = dst;
   pld(start, 0);
   pld(mask, 0);
   end = start + (size & ~3);

   while (start < end)
     {
        pld(start, 8);
        pld(mask, 4);
        UNROLL4(
                  {
                  _glyph_pt_mask_transp_solid(start, gry8, rel_alpha, mask);
                  start++; mask++;}
        );
     }

   end = start + (size & 3);
   for (; start < end; start++, mask++)
      _glyph_pt_mask_transp_solid(start, gry8, rel_alpha, mask);
}

static inline void
_calc_ext(const Soft8_Image * dst, const RGBA_Draw_Context * dc,
          Eina_Rectangle * ext)
{
   EINA_RECTANGLE_SET(ext, 0, 0, dst->cache_entry.w, dst->cache_entry.h);

   if (dc->clip.use)
     {
        int v;

        EINA_RECTANGLE_SET(ext, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
        if (ext->x < 0)
          {
             ext->w += ext->x;
             ext->x = 0;
          }
        if (ext->y < 0)
          {
             ext->h += ext->y;
             ext->y = 0;
          }

        v = dst->cache_entry.w - ext->x;
        if (ext->w > v)
           ext->w = v;

        v = dst->cache_entry.h - ext->y;
        if (ext->h > v)
           ext->h = v;
     }
}

static inline void
_glyph_scanline(Soft8_Image * dst, const DATA8 * p_mask,
                const Eina_Rectangle ext, int dx, int dy, int max_x, int max_y,
                int w, DATA8 alpha, const DATA8 gry8)
{
   int size, in_x, in_w;
   DATA8 *p_pixels;

   if ((dx >= max_x) || (dy < ext.y) || (dy >= max_y))
      return;

   in_x = 0;
   in_w = 0;

   if (dx + w > max_x)
      in_w += (dx + w) - max_x;

   if (dx < ext.x)
     {
        in_w += ext.x - dx;
        in_x = ext.x - dx;
        dx = ext.x;
     }

   size = w - in_w;
   p_pixels = dst->pixels + (dy * dst->stride) + dx;
   p_mask += in_x;

   if (size > 1)
     {
        if (alpha == 0xff)
           _glyph_scanline_mask_solid_solid(p_pixels, size, gry8, p_mask);
        else if (alpha != 0)
           _glyph_scanline_mask_transp_solid
               (p_pixels, size, gry8, alpha, p_mask);
     }
   else if (size == 1)
     {
        if (alpha == 0xff)
           _glyph_pt_mask_solid_solid(p_pixels, gry8, p_mask);
        else if (alpha != 0)
           _glyph_pt_mask_transp_solid(p_pixels, gry8, alpha, p_mask);
     }
}

static void
_soft8_font_glyph_draw_grayscale(Soft8_Image * dst,
                                 RGBA_Draw_Context * dc __UNUSED__,
                                 RGBA_Font_Glyph * fg __UNUSED__, int x, int y,
                                 DATA8 alpha, DATA8 gry8,
                                 const Eina_Rectangle ext, int bw, int bh,
                                 int bpitch, const DATA8 * bitmap)
{
   int i, max_x, max_y;

   max_x = ext.x + ext.w;
   max_y = ext.y + ext.h;

   for (i = 0; i < bh; i++, bitmap += bpitch)
      _glyph_scanline(dst, bitmap, ext, x, y + i, max_x, max_y, bw,
                      alpha, gry8);
}

static inline void
_glyph_create_mask_line(DATA8 * mask, const DATA8 * bitmap, int w)
{
   const DATA8 bitrepl[2] = { 0x0, 0xff };
   int i;

   for (i = 0; i < w; i += 8, bitmap++)
     {
        int j, size;
        DATA32 bits;

        if (i + 8 < w)
           size = 8;
        else
           size = w - i;

        bits = *bitmap;

        for (j = size - 1; j >= 0; j--, mask++)
           *mask = bitrepl[(bits >> j) & 0x1];
     }
}

static void
_soft8_font_glyph_draw_mono(Soft8_Image * dst,
                            RGBA_Draw_Context * dc __UNUSED__,
                            RGBA_Font_Glyph * fg __UNUSED__, int x, int y,
                            DATA8 alpha, DATA8 gry8, const Eina_Rectangle ext,
                            int bw, int bh, int bpitch, const DATA8 * bitmap)
{
   DATA8 *mask;
   int i, max_x, max_y;

   max_x = ext.x + ext.w;
   max_y = ext.y + ext.h;

   mask = alloca(bpitch);
   for (i = 0; i < bh; i++, bitmap += bpitch)
     {
        _glyph_create_mask_line(mask, bitmap, bw);
        _glyph_scanline(dst, mask, ext, x, y + i, max_x, max_y, bw,
                        alpha, gry8);
     }
}

void
evas_common_soft8_font_glyph_draw(void *data, void *dest __UNUSED__, void *context,
                      RGBA_Font_Glyph * fg, int x, int y)
{
   Soft8_Image *dst;
   RGBA_Draw_Context *dc;
   const DATA8 *bitmap;
   DATA8 alpha; // r, g, b
   DATA8 gry8;
   Eina_Rectangle ext;
   int bpitch, bw, bh;

   dst = data;
   dc = context;

   alpha = A_VAL(&dc->col.col);
   if (alpha == 0)
      return;

   gry8 = GRY_8_FROM_RGB(&dc->col.col);

   /*
    * if (r > alpha) r = alpha;
    * if (g > alpha) g = alpha;
    * if (b > alpha) b = alpha;
    *
    * gry8 = GRY_8_FROM_COMPONENTS(r, g, b);
    */

   bitmap = fg->glyph_out->bitmap.buffer;
   bh = fg->glyph_out->bitmap.rows;
   bw = fg->glyph_out->bitmap.width;
   bpitch = fg->glyph_out->bitmap.pitch;
   if (bpitch < bw)
      bpitch = bw;

   _calc_ext(dst, dc, &ext);

   if ((fg->glyph_out->bitmap.num_grays == 256) &&
       (fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays))
      _soft8_font_glyph_draw_grayscale(dst, dc, fg, x, y, alpha, gry8,
                                       ext, bw, bh, bpitch, bitmap);
   else
      _soft8_font_glyph_draw_mono(dst, dc, fg, x, y, alpha, gry8,
                                  ext, bw, bh, bpitch, bitmap);
}

void *
evas_common_soft8_font_glyph_new(void *data __UNUSED__, RGBA_Font_Glyph * fg __UNUSED__)
{
   return (void *)1;            /* core requires != NULL to work */
}

void
evas_common_soft8_font_glyph_free(void *ext_dat __UNUSED__)
{
}
