/* @file blur_gaussian_rgba_.c
 * Should define the functions:
 * - _gaussian_blur_horiz_rgba_step
 * - _gaussian_blur_vert_rgba_step
 */

#include "evas_common_private.h"
#include "../evas_filter_private.h"

#if !defined (FUNCTION_NAME) || !defined (STEP)
# error Must define FUNCTION_NAME and STEP
#endif

static inline void
FUNCTION_NAME(const DATA32* restrict srcdata, DATA32* restrict dstdata,
              const int radius, const int len,
              const int loops, const int loopstep,
              const int* restrict weights, const int pow2_divider)
{
   const int diameter = 2 * radius + 1;
   const int left = MIN(radius, len);
   const int right = MIN(radius, (len - radius));
   const DATA32* restrict src;
   DATA32* restrict dst;
   int i, j, k;

   for (i = loops; i; --i)
     {
        src = srcdata;
        dst = dstdata;

        // left
        for (k = 0; k < left; k++, dst += STEP)
          {
             int acc[4] = {0};
             int divider = 0;
             const DATA32* restrict s = src;
             for (j = 0; j <= k + radius; j++, s += STEP)
               {
                  const int weightidx = j + radius - k;
                  acc[ALPHA] += A_VAL(s) * weights[weightidx];
                  acc[RED]   += R_VAL(s) * weights[weightidx];
                  acc[GREEN] += G_VAL(s) * weights[weightidx];
                  acc[BLUE]  += B_VAL(s) * weights[weightidx];
                  divider += weights[weightidx];
               }
             //if (!divider) abort();
             A_VAL(dst) = acc[ALPHA] / divider;
             R_VAL(dst) = acc[RED]   / divider;
             G_VAL(dst) = acc[GREEN] / divider;
             B_VAL(dst) = acc[BLUE]  / divider;
          }

        // middle
        for (k = len - (2 * radius); k > 0; k--, src += STEP, dst += STEP)
          {
             int acc[4] = {0};
             const DATA32* restrict s = src;
             for (j = 0; j < diameter; j++, s += STEP)
               {
                  acc[ALPHA] += A_VAL(s) * weights[j];
                  acc[RED]   += R_VAL(s) * weights[j];
                  acc[GREEN] += G_VAL(s) * weights[j];
                  acc[BLUE]  += B_VAL(s) * weights[j];
               }
             A_VAL(dst) = acc[ALPHA] >> pow2_divider;
             R_VAL(dst) = acc[RED]   >> pow2_divider;
             G_VAL(dst) = acc[GREEN] >> pow2_divider;
             B_VAL(dst) = acc[BLUE]  >> pow2_divider;
          }

        // right
        for (k = 0; k < right; k++, dst += STEP, src += STEP)
          {
             int acc[4] = {0};
             int divider = 0;
             const DATA32* restrict s = src;
             for (j = 0; j < 2 * radius - k; j++, s += STEP)
               {
                  acc[ALPHA] += A_VAL(s) * weights[j];
                  acc[RED]   += R_VAL(s) * weights[j];
                  acc[GREEN] += G_VAL(s) * weights[j];
                  acc[BLUE]  += B_VAL(s) * weights[j];
                  divider += weights[j];
               }
             //if (!divider) abort();
             A_VAL(dst) = acc[ALPHA] / divider;
             R_VAL(dst) = acc[RED]   / divider;
             G_VAL(dst) = acc[GREEN] / divider;
             B_VAL(dst) = acc[BLUE]  / divider;
          }

        dstdata += loopstep;
        srcdata += loopstep;
     }
}

#undef FUNCTION_NAME
#undef STEP
