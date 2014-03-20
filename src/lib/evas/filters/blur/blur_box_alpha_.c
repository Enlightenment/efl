/* @file blur_box_alpha_.c
 * Defines the following functions:
 * _box_blur_alpha_horiz_step
 * _box_blur_alpha_vert_step
 */

#include "evas_common_private.h"
#include "../evas_filter_private.h"

static inline void
_box_blur_alpha_horiz_step(const DATA8* restrict const srcdata,
                           DATA8* restrict const dstdata,
                           const int* restrict const radii,
                           const int len,
                           const int loops)
{
   const DATA8* restrict src;
   DATA8* restrict dst;
   DATA8* restrict span1;
   DATA8* restrict span2;

#if DIV_USING_BITSHIFT
   int pow2_shifts[6] = {0};
   int numerators[6] = {0};
   for (int run = 0; radii[run]; run++)
     {
        const int div = radii[run] * 2 + 1;
        pow2_shifts[run] = evas_filter_smallest_pow2_larger_than(div << 10);
        numerators[run] = (1 << pow2_shifts[run]) / (div);
     }
#endif

   span1 = alloca(len);
   span2 = alloca(len);
   memset(span1, 0, len);
   memset(span2, 0, len);

   // For each line, apply as many blurs as requested
   for (int l = 0; l < loops; l++)
     {
        int run;

        // New line: reset source & destination pointers
        src = srcdata + len * l;
        if (!radii[1]) // Only one run
          dst = dstdata + len * l;
        else
          dst = span1;

        // Apply blur with current radius
        for (run = 0; radii[run]; run++)
          {
             const int radius = radii[run];
             const int left = MIN(radius, len);

#if DIV_USING_BITSHIFT
             const int pow2 = pow2_shifts[run];
             const int numerator = numerators[run];
#else
             const int divider = 2 * radius + 1;
#endif

             const DATA8* restrict sl = src;
             const DATA8* restrict sr = src;
             const DATA8* restrict sre = src + len;
             const DATA8* restrict sle = src + len - radius;
             DATA8* restrict d = dst;
             int acc = 0, count = 0;

             // Read-ahead & accumulate
             for (int x = left; x > 0; x--)
               {
                  acc += *sr++;
                  count++;
               }

             // Left edge
             for (int x = left; x > 0; x--)
               {
                  if (sr < sre)
                    {
                       acc += *sr++;
                       count++;
                    }

                  *d++ = acc / count;
               }

             // Middle part, normal blur
             while (sr < sre)
               {
                  acc += *sr++;
                  *d++ = DIVIDE(acc);
                  acc -= *sl++;
               }

             // Right edge
             count = 2 * radius + 1;
             while (sl < sle)
               {
                  *d++ = acc / (--count);
                  acc -= *sl++;
               }

             // More runs to go: swap spans
             if (radii[run + 1])
               {
                  src = dst;
                  if (radii[run + 2])
                    {
                       // Two more runs: swap
                       DATA8* swap = span1;
                       span1 = span2;
                       span2 = swap;
                       dst = span1;
                    }
                  else
                    {
                       // Last run: write directly to dstdata
                       dst = dstdata + len * l;
                    }
               }
          }
     }
}

// ATTENTION: Make sure the below code's inner loop is the SAME as above.

static inline void
_box_blur_alpha_vert_step(const DATA8* restrict const srcdata,
                          DATA8* restrict const dstdata,
                          const int* restrict const radii,
                          const int len,
                          const int loops)
{
   /* Note: This function tries to optimize cache hits by working on
    * contiguous horizontal spans.
    */

   const int step = loops;
   DATA8* restrict src;
   DATA8* restrict dst;
   DATA8* restrict span1;
   DATA8* restrict span2;

#if DIV_USING_BITSHIFT
   int pow2_shifts[6] = {0};
   int numerators[6] = {0};
   for (int run = 0; radii[run]; run++)
     {
        const int div = radii[run] * 2 + 1;
        pow2_shifts[run] = evas_filter_smallest_pow2_larger_than(div << 10);
        numerators[run] = (1 << pow2_shifts[run]) / (div);
     }
#endif

   span1 = alloca(len);
   span2 = alloca(len);

   // For each line, apply as many blurs as requested
   for (int l = 0; l < loops; l++)
     {
        int run;

        // Rotate input into work span
        const DATA8* srcptr = srcdata + l;
        DATA8* s = span1;
        for (int k = len; k; --k)
          {
             *s++ = *srcptr;
             srcptr += step;
          }

        src = span1;
        dst = span2;

        // Apply blur with current radius
        for (run = 0; radii[run]; run++)
          {
             const int radius = radii[run];
             const int left = MIN(radius, len);

#if DIV_USING_BITSHIFT
             const int pow2 = pow2_shifts[run];
             const int numerator = numerators[run];
#else
             const int divider = 2 * radius + 1;
#endif

             const DATA8* restrict sl = src;
             const DATA8* restrict sr = src;
             const DATA8* restrict sre = src + len;
             const DATA8* restrict sle = src + len - radius;
             DATA8* restrict d = dst;
             int acc = 0, count = 0;

             // Read-ahead & accumulate
             for (int x = left; x > 0; x--)
               {
                  acc += *sr++;
                  count++;
               }

             // Left edge
             for (int x = left; x > 0; x--)
               {
                  if (sr < sre)
                    {
                       acc += *sr++;
                       count++;
                    }

                  *d++ = acc / count;
               }

             // Middle part, normal blur
             while (sr < sre)
               {
                  acc += *sr++;
                  *d++ = DIVIDE(acc);
                  acc -= *sl++;
               }

             // Right edge
             count = 2 * radius + 1;
             while (sl < sle)
               {
                  *d++ = acc / (--count);
                  acc -= *sl++;
               }

             // More runs to go: swap spans
             if (radii[run + 1])
               {
                  DATA8* swap = src;
                  src = dst;
                  dst = swap;
               }
          }

        // Last run: rotate & copy back to destination
        DATA8* restrict dstptr = dstdata + l;
        for (int k = len; k; --k)
          {
             *dstptr = *dst++;
             dstptr += step;
          }
     }
}
