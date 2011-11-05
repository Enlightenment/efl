#include "evas_common_soft8.h"

EFL_ALWAYS_INLINE void
_soft8_convert_from_rgba_pt(const DATA32 * src, DATA8 * dst, DATA8 * alpha)
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
                                  DATA8 * alpha, const int w)
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
                  _soft8_convert_from_rgba_pt(src, dst, alpha);
                  src++; dst++; alpha++; x++;}
        );
     }

   for (; x < w; x++, src++, dst++, alpha++)
      _soft8_convert_from_rgba_pt(src, dst, alpha);
}

void
evas_common_soft8_image_convert_from_rgba(Soft8_Image * im, const DATA32 * src)
{
   const DATA32 *sp;
   DATA8 *dp;
   DATA8 *ap;
   unsigned int y;

   sp = src;
   dp = im->pixels;
   ap = im->alpha;

   for (y = 0; y < im->cache_entry.h;
        y++, sp += im->cache_entry.w, dp += im->stride, ap += im->stride)
      _soft8_convert_from_rgba_scanline(sp, dp, ap, im->cache_entry.w);
}

EFL_ALWAYS_INLINE void
_soft8_convert_from_rgb_pt(const DATA32 * src, DATA8 * dst)
{
   *dst = GRY_8_FROM_RGB(src);
}

static inline void
_soft8_convert_from_rgb_scanline(const DATA32 * src, DATA8 * dst, const int w)
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
                  _soft8_convert_from_rgb_pt(src, dst); src++; dst++; x++;}
        );
     }

   for (; x < w; x++, src++, dst++)
      _soft8_convert_from_rgb_pt(src, dst);
}

void
evas_common_soft8_image_convert_from_rgb(Soft8_Image * im, const DATA32 * src)
{
   const DATA32 *sp;
   DATA8 *dp;
   unsigned int y;

   sp = src;
   dp = im->pixels;

   for (y = 0; y < im->cache_entry.h;
        y++, sp += im->cache_entry.w, dp += im->stride)
      _soft8_convert_from_rgb_scanline(sp, dp, im->cache_entry.w);
}
