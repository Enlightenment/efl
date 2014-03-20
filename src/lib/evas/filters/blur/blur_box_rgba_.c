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
   memset(span1, 0, len * sizeof(DATA32));
   memset(span2, 0, len * sizeof(DATA32));

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

             const DATA8* restrict sl = (DATA8 *) src;
             const DATA8* restrict sr = (DATA8 *) src;
             const DATA8* restrict sre = (DATA8 *) (src + len);
             const DATA8* restrict sle = (DATA8 *) (src + len - radius);
             DATA8* restrict d = (DATA8 *) dst;
             int acc[4] = {0};
             int count = 0;

             // Read-ahead
             for (int x = left; x > 0; x--)
               {
                  for (int k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);
                  count++;
               }

             // Left
             for (int x = left; x > 0; x--)
               {
                  if (sr < sre)
                    {
                       for (int k = 0; k < 4; k++)
                         acc[k] += sr[k];
                       sr += sizeof(DATA32);
                       count++;
                    }

                  d[ALPHA] = acc[ALPHA] / count;
                  d[RED]   = acc[RED]   / count;
                  d[GREEN] = acc[GREEN] / count;
                  d[BLUE]  = acc[BLUE]  / count;
                  d += sizeof(DATA32);
               }

             // Main part
             for (; sr < sre; sr += sizeof(DATA32), sl += sizeof(DATA32))
               {
                  for (int k = 0; k < 4; k++)
                    acc[k] += sr[k];

                  d[ALPHA] = DIVIDE(acc[ALPHA]);
                  d[RED]   = DIVIDE(acc[RED]);
                  d[GREEN] = DIVIDE(acc[GREEN]);
                  d[BLUE]  = DIVIDE(acc[BLUE]);
                  d += sizeof(DATA32);

                  for (int k = 0; k < 4; k++)
                    acc[k] -= sl[k];
               }

             // Right part
             count = 2 * radius + 1;
             for (; sl < sle; sl += sizeof(DATA32))
               {
                  const int divider = --count;
                  d[ALPHA] = acc[ALPHA] / divider;
                  d[RED]   = acc[RED]   / divider;
                  d[GREEN] = acc[GREEN] / divider;
                  d[BLUE]  = acc[BLUE]  / divider;
                  d += sizeof(DATA32);

                  for (int k = 0; k < 4; k++)
                    acc[k] -= sl[k];
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
   memset(span1, 0, len * sizeof(DATA32));
   memset(span2, 0, len * sizeof(DATA32));

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

#if DIV_USING_BITSHIFT
             const int pow2 = pow2_shifts[run];
             const int numerator = numerators[run];
#else
             const int divider = 2 * radius + 1;
#endif

             const DATA8* restrict sl = (DATA8 *) src;
             const DATA8* restrict sr = (DATA8 *) src;
             const DATA8* restrict sre = (DATA8 *) (src + len);
             const DATA8* restrict sle = (DATA8 *) (src + len - radius);
             DATA8* restrict d = (DATA8 *) dst;
             int acc[4] = {0};
             int count = 0;

             // Read-ahead
             for (int x = left; x > 0; x--)
               {
                  for (int k = 0; k < 4; k++)
                    acc[k] += sr[k];
                  sr += sizeof(DATA32);
                  count++;
               }

             // Left
             for (int x = left; x > 0; x--)
               {
                  if (sr < sre)
                    {
                       for (int k = 0; k < 4; k++)
                         acc[k] += sr[k];
                       sr += sizeof(DATA32);
                       count++;
                    }

                  d[ALPHA] = acc[ALPHA] / count;
                  d[RED]   = acc[RED]   / count;
                  d[GREEN] = acc[GREEN] / count;
                  d[BLUE]  = acc[BLUE]  / count;
                  d += sizeof(DATA32);
               }

             // Main part
             for (; sr < sre; sr += sizeof(DATA32), sl += sizeof(DATA32))
               {
                  for (int k = 0; k < 4; k++)
                    acc[k] += sr[k];

                  d[ALPHA] = DIVIDE(acc[ALPHA]);
                  d[RED]   = DIVIDE(acc[RED]);
                  d[GREEN] = DIVIDE(acc[GREEN]);
                  d[BLUE]  = DIVIDE(acc[BLUE]);
                  d += sizeof(DATA32);

                  for (int k = 0; k < 4; k++)
                    acc[k] -= sl[k];
               }

             // Right part
             count = 2 * radius + 1;
             for (; sl < sle; sl += sizeof(DATA32))
               {
                  const int divider = --count;
                  d[ALPHA] = acc[ALPHA] / divider;
                  d[RED]   = acc[RED]   / divider;
                  d[GREEN] = acc[GREEN] / divider;
                  d[BLUE]  = acc[BLUE]  / divider;
                  d += sizeof(DATA32);

                  for (int k = 0; k < 4; k++)
                    acc[k] -= sl[k];
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
