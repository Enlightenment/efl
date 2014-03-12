/* @file blur_box_rgba_.c
 * Should define the functions:
 * - _box_blur_horiz_rgba_step
 * - _box_blur_vert_rgba_step
 */

#include "evas_common_private.h"
#include "../evas_filter_private.h"

static inline void
_box_blur_horiz_rgba_step(const DATA32* restrict const srcdata,
                          DATA32* restrict const dstdata,
                          const int* restrict const radii,
                          const int len,
                          const int loops)
{
   const DATA32* restrict src;
   DATA32* restrict dst;
   DATA32* restrict span1;
   DATA32* restrict span2;

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

   span1 = alloca(len * sizeof(DATA32));
   span2 = alloca(len * sizeof(DATA32));

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
             const int right = MIN(radius, (len - radius));

#if DIV_USING_BITSHIFT
             const int pow2 = pow2_shifts[run];
             const int numerator = numerators[run];
#else
             const int divider = 2 * radius + 1;
#endif

             const DATA8* restrict sl = (DATA8 *) src;
             const DATA8* restrict sr = (DATA8 *) src;
             DATA8* restrict d = (DATA8 *) dst;
             int acc[4] = {0};
             int x, k;

             // Read-ahead
             for (x = left; x; x--)
               {
                  for (k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);
               }

             // Left
             for (x = 0; x < left; x++)
               {
                  for (k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);

                  const int divider = x + left + 1;
                  d[ALPHA] = acc[ALPHA] / divider;
                  d[RED]   = acc[RED]   / divider;
                  d[GREEN] = acc[GREEN] / divider;
                  d[BLUE]  = acc[BLUE]  / divider;
                  d += sizeof(DATA32);
               }

             // Main part
             for (x = len - (2 * radius); x > 0; x--)
               {
                  for (k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);

                  d[ALPHA] = DIVIDE(acc[ALPHA]);
                  d[RED]   = DIVIDE(acc[RED]);
                  d[GREEN] = DIVIDE(acc[GREEN]);
                  d[BLUE]  = DIVIDE(acc[BLUE]);
                  d += sizeof(DATA32);

                  for (k = 0; k < 4; k++)
                    acc[k] -= sl[k];
                  sl += sizeof(DATA32);
               }

             // Right part
             for (x = right; x; x--)
               {
                  const int divider = x + right;
                  d[ALPHA] = acc[ALPHA] / divider;
                  d[RED]   = acc[RED]   / divider;
                  d[GREEN] = acc[GREEN] / divider;
                  d[BLUE]  = acc[BLUE]  / divider;
                  d += sizeof(DATA32);

                  for (k = 0; k < 4; k++)
                    acc[k] -= sl[k];
                  sl += sizeof(DATA32);
               }

             // More runs to go: swap spans
             if (radii[run + 1])
               {
                  src = dst;
                  if (radii[run + 2])
                    {
                       // Two more runs: swap
                       DATA32* swap = span1;
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

static inline void
_box_blur_vert_rgba_step(const DATA32* restrict const srcdata,
                         DATA32* restrict const dstdata,
                         const int* restrict const radii,
                         const int len,
                         const int loops)
{
   /* Note: This function tries to optimize cache hits by working on
    * contiguous horizontal spans.
    */

   const int step = loops;
   DATA32* restrict src;
   DATA32* restrict dst;
   DATA32* restrict span1;
   DATA32* restrict span2;

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

   span1 = alloca(len * sizeof(DATA32));
   span2 = alloca(len * sizeof(DATA32));

   // For each line, apply as many blurs as requested
   for (int l = 0; l < loops; l++)
     {
        int run;

        // Rotate input into work span
        const DATA32* srcptr = srcdata + l;
        DATA32* s = span1;
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
             const int right = MIN(radius, (len - radius));

#if DIV_USING_BITSHIFT
             const int pow2 = pow2_shifts[run];
             const int numerator = numerators[run];
#else
             const int divider = 2 * radius + 1;
#endif

             const DATA8* restrict sl = (DATA8 *) src;
             const DATA8* restrict sr = (DATA8 *) src;
             DATA8* restrict d = (DATA8 *) dst;
             int acc[4] = {0};
             int x, k;

             // Read-ahead
             for (x = left; x; x--)
               {
                  for (k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);
               }

             // Left
             for (x = 0; x < left; x++)
               {
                  for (k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);

                  const int divider = x + left + 1;
                  d[ALPHA] = acc[ALPHA] / divider;
                  d[RED]   = acc[RED]   / divider;
                  d[GREEN] = acc[GREEN] / divider;
                  d[BLUE]  = acc[BLUE]  / divider;
                  d += sizeof(DATA32);
               }

             // Main part
             for (x = len - (2 * radius); x > 0; x--)
               {
                  for (k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);

                  d[ALPHA] = DIVIDE(acc[ALPHA]);
                  d[RED]   = DIVIDE(acc[RED]);
                  d[GREEN] = DIVIDE(acc[GREEN]);
                  d[BLUE]  = DIVIDE(acc[BLUE]);
                  d += sizeof(DATA32);

                  for (k = 0; k < 4; k++)
                    acc[k] -= sl[k];
                  sl += sizeof(DATA32);
               }

             // Right part
             for (x = right; x; x--)
               {
                  const int divider = x + right;
                  d[ALPHA] = acc[ALPHA] / divider;
                  d[RED]   = acc[RED]   / divider;
                  d[GREEN] = acc[GREEN] / divider;
                  d[BLUE]  = acc[BLUE]  / divider;
                  d += sizeof(DATA32);

                  for (k = 0; k < 4; k++)
                    acc[k] -= sl[k];
                  sl += sizeof(DATA32);
               }

             // More runs to go: swap spans
             if (radii[run + 1])
               {
                  DATA32* swap = src;
                  src = dst;
                  dst = swap;
               }
          }

        // Last run: rotate & copy back to destination
        DATA32* restrict dstptr = dstdata + l;
        for (int k = len; k; --k)
          {
             *dstptr = *dst++;
             dstptr += step;
          }
     }
}
