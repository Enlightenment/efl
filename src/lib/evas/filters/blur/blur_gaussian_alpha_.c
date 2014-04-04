/* @file blur_gaussian_alpha_.c
 * Should define the functions:
 * - _gaussian_blur_horiz_alpha_step
 * - _gaussian_blur_vert_alpha_step
 */

/* Datatypes and MIN macro */
#include "../evas_filter_private.h"

#if !defined (FUNCTION_NAME) || !defined (STEP)
# error Must define FUNCTION_NAME and STEP
#endif

static inline void
FUNCTION_NAME(const DATA8* restrict srcdata, DATA8* restrict dstdata,
              const int radius, const int len,
              const int loops, const int loopstep,
              const int* restrict weights, const int pow2_divider)
{
   int i, j, k, acc, divider;
   const int diameter = 2 * radius + 1;
   const int left = MIN(radius, len);
   const int right = MIN(radius, (len - radius));
   const DATA8* restrict s;
   const DATA8* restrict src;
   DATA8* restrict dst;

   for (i = loops; i; --i)
     {
        src = srcdata;
        dst = dstdata;

        // left
        for (k = 0; k < left; k++, dst += STEP)
          {
             acc = 0;
             divider = 0;
             s = src;
             for (j = 0; j <= k + radius; j++, s += STEP)
               {
                  acc += (*s) * weights[j + radius - k];
                  divider += weights[j + radius - k];
               }
             //if (!divider) abort();
             *dst = acc / divider;
          }

        // middle
        for (k = radius; k < (len - radius); k++, src += STEP, dst += STEP)
          {
             acc = 0;
             s = src;
             for (j = 0; j < diameter; j++, s += STEP)
               acc += (*s) * weights[j];
             *dst = acc >> pow2_divider;
          }

        // right
        for (k = 0; k < right; k++, dst += STEP, src += STEP)
          {
             acc = 0;
             divider = 0;
             s = src;
             for (j = 0; j < 2 * radius - k; j++, s += STEP)
               {
                  acc += (*s) * weights[j];
                  divider += weights[j];
               }
             //if (!divider) abort();
             *dst = acc / divider;
          }

        dstdata += loopstep;
        srcdata += loopstep;
     }
}

#undef FUNCTION_NAME
#undef STEP
