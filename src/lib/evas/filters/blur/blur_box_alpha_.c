/* @file blur_box_alpha_.c
 * Should define the functions:
 * - _box_blur_horiz_alpha_step
 * - _box_blur_vert_alpha_step
 */

#include "evas_common_private.h"
#include "../evas_filter_private.h"

#if !defined (FUNCTION_NAME) || !defined (STEP)
# error Must define FUNCTION_NAME and STEP
#endif

static inline void
FUNCTION_NAME(const DATA8* restrict src, DATA8* restrict dst,
              const int radius, const int len,
              const int loops, const int loopstep)
{
   DEFINE_DIVIDER(2 * radius + 1);
   const int left = MIN(radius, len);
   const int right = MIN(radius, (len - radius));
   int acc = 0, k;

   for (int l = loops; l; --l)
     {
        const DATA8* restrict sr = src;
        const DATA8* restrict sl = src;
        DATA8* restrict d = dst;

        for (k = left; k; k--)
          {
             acc += *sr;
             sr += STEP;
          }

        for (k = 0; k < left; k++)
          {
             acc += *sr;
             *d = acc / (k + left + 1);
             sr += STEP;
             d += STEP;
          }

        for (k = len - (2 * radius); k; k--)
          {
             acc += *sr;
             *d = DIVIDE(acc);
             acc -= *sl;
             sl += STEP;
             sr += STEP;
             d += STEP;
          }

        for (k = right; k; k--)
          {
             *d = acc / (k + right);
             acc -= *sl;
             d += STEP;
             sl += STEP;
          }

        src += loopstep;
        dst += loopstep;
     }
}

#undef FUNCTION_NAME
#undef STEP
