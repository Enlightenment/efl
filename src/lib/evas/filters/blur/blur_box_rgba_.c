/* @file blur_box_rgba_.c
 * Should define the functions:
 * - _box_blur_horiz_rgba_step
 * - _box_blur_vert_rgba_step
 */

#include "evas_common_private.h"
#include "../evas_filter_private.h"

#if !defined (FUNCTION_NAME) || !defined (STEP)
# error Must define FUNCTION_NAME and STEP
#endif

static inline void
FUNCTION_NAME(const DATA32* restrict src, DATA32* restrict dst,
              const int radius, const int len,
              const int loops, const int loopstep)
{
   DEFINE_DIVIDER(2 * radius + 1);
   const int left = MIN(radius, len);
   const int right = MIN(radius, (len - radius));

   for (int l = loops; l; --l)
     {
        int acc[4] = {0};
        int x, k;
        int divider;

        const DATA8* restrict sl = (DATA8 *) src;
        const DATA8* restrict sr = (DATA8 *) src;
        DATA8* restrict d = (DATA8 *) dst;

        // Read-ahead
        for (x = left; x; x--)
          {
             for (k = 0; k < 4; k++)
               acc[k] += sr[k];
             sr += STEP;
          }

        // Left
        for (x = 0; x < left; x++)
          {
             for (k = 0; k < 4; k++)
               acc[k] += sr[k];
             sr += STEP;

             divider = x + left + 1;
             d[ALPHA] = acc[ALPHA] / divider;
             d[RED]   = acc[RED]   / divider;
             d[GREEN] = acc[GREEN] / divider;
             d[BLUE]  = acc[BLUE]  / divider;
             d += STEP;
          }

        // Main part
        for (x = len - (2 * radius); x > 0; x--)
          {
             for (k = 0; k < 4; k++)
               acc[k] += sr[k];
             sr += STEP;

             d[ALPHA] = DIVIDE(acc[ALPHA]);
             d[RED]   = DIVIDE(acc[RED]);
             d[GREEN] = DIVIDE(acc[GREEN]);
             d[BLUE]  = DIVIDE(acc[BLUE]);
             d += STEP;

             for (k = 0; k < 4; k++)
               acc[k] -= sl[k];
             sl += STEP;
          }

        // Right part
        for (x = right; x; x--)
          {
             divider = x + right;
             d[ALPHA] = acc[ALPHA] / divider;
             d[RED]   = acc[RED]   / divider;
             d[GREEN] = acc[GREEN] / divider;
             d[BLUE]  = acc[BLUE]  / divider;
             d += STEP;

             for (k = 0; k < 4; k++)
               acc[k] -= sl[k];
             sl += STEP;
          }

        src += loopstep;
        dst += loopstep;
     }
}

#undef FUNCTION_NAME
#undef STEP
