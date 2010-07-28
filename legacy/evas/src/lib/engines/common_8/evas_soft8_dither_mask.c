#include "evas_common_soft8.h"

static always_inline void
_soft8_convert_from_rgba_pt(const DATA32 * src, DATA8 * dst, DATA8 * alpha,
                            const int x, const int y)
{
   if (A_VAL(src) == 0)
     {
        *dst = 0;
        *alpha = 0;
     }
   else
     {
        *dst = GRY_8_FROM_RGB(src);
        *alpha = A_VAL(src);
     }
}

static inline void
_soft8_convert_from_rgba_scanline(const DATA32 * src, DATA8 * dst,
                                  DATA8 * alpha, const int y, const int w)
{
   int x, m;

   m = (w & ~7);
   x = 0;
   pld(src, 0);

   while (x < m)
     {
        pld(src, 32);
        UNROLL8(
                  {
                  _soft8_convert_from_rgba_pt(src, dst, alpha, x, y);
                  src++; dst++; alpha++; x++;}
        );
     }

   for (; x < w; x++, src++, dst++, alpha++)
      _soft8_convert_from_rgba_pt(src, dst, alpha, x, y);
}

void
soft8_image_convert_from_rgba(Soft8_Image * im, const DATA32 * src)
{
   const DATA32 *sp;
   DATA8 *dp;
   DATA8 *ap;
   int y;

   sp = src;
   dp = im->pixels;
   ap = im->alpha;

   for (y = 0; y < im->cache_entry.h;
        y++, sp += im->cache_entry.w, dp += im->stride, ap += im->stride)
      _soft8_convert_from_rgba_scanline(sp, dp, ap, y, im->cache_entry.w);
}

static always_inline void
_soft8_convert_from_rgb_pt(const DATA32 * src, DATA8 * dst,
                           const int x, const int y)
{
   *dst = GRY_8_FROM_RGB(src);
}

static inline void
_soft8_convert_from_rgb_scanline(const DATA32 * src, DATA8 * dst, const int y,
                                 const int w)
{
   int x, m;

   m = (w & ~7);
   x = 0;
   pld(src, 0);

   while (x < m)
     {
        pld(src, 32);
        UNROLL8(
                  {
                  _soft8_convert_from_rgb_pt(src, dst, x, y); src++; dst++; x++;}
        );
     }

   for (; x < w; x++, src++, dst++)
      _soft8_convert_from_rgb_pt(src, dst, x, y);
}

void
soft8_image_convert_from_rgb(Soft8_Image * im, const DATA32 * src)
{
   const DATA32 *sp;
   DATA8 *dp;
   int y;

   sp = src;
   dp = im->pixels;

   for (y = 0; y < im->cache_entry.h;
        y++, sp += im->cache_entry.w, dp += im->stride)
      _soft8_convert_from_rgb_scanline(sp, dp, y, im->cache_entry.w);
}
