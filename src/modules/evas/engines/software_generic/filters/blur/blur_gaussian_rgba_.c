/* @file blur_gaussian_rgba_.c
 * Should define the functions:
 * - _gaussian_blur_horiz_rgba_step
 * - _gaussian_blur_vert_rgba_step
 *
 * Define BLUR_NEON as well to get the same kernel with its middle section
 * vectorised. There is deliberately no separate NEON copy of this file: the
 * ramps are subtle enough that two copies would drift, and a differential test
 * can only compare what the two copies still agree to compute.
 *
 * Which axis the middle can be vectorised along depends on the direction,
 * because the two instantiations disagree about what has stride one:
 *
 *   horizontal (STEP == 1): the taps of one output are contiguous, so walk the
 *              line four pixels at a time, one unaligned load per tap.
 *
 *   vertical (STEP == loops): one output's window strides a row per tap, but
 *              adjacent *columns* are contiguous. That axis belongs to the
 *              caller's outer loop, so the middle is done in a separate pass
 *              that inverts the loop nest, and the per line loop below then
 *              only fills in the ramps for the columns it covered.
 *
 * Accumulators are 32 bit: the largest value is 255 * sum(weights), and the
 * weights are normalised so that sum(weights) == 1 << pow2_divider, which for
 * any radius evas uses stays far inside 32 bits. The weights themselves are a
 * few thousand at most, so they fit the 16 bit scalar operand of vmlal_n_u16.
 */

#include "evas_filter_private.h"

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
   const int mid = len - (2 * radius);
   /* where the right ramp starts reading, i.e. how far the middle advanced */
   const int roff = (mid > 0) ? mid : 0;
   const DATA32* restrict src;
   DATA32* restrict dst;
   int i, j, k;
#ifdef BLUR_NEON
   const int32x4_t shift = vdupq_n_s32(-pow2_divider);
   int done_cols = 0;

   /* Vertical only: the middle of every column, four columns at a time. Each
    * tap is one contiguous 16 byte load spanning four columns of one row. */
   if ((STEP != 1) && (mid > 0))
     {
        int c;

        for (c = 0; c + 4 <= loops; c += 4)
          {
             const DATA32* restrict scol = srcdata + c;
             DATA32* restrict dcol = dstdata + c + (size_t) left * STEP;

             for (k = 0; k < mid; k++, scol += STEP, dcol += STEP)
               {
                  uint32x4_t a0 = vdupq_n_u32(0), a1 = vdupq_n_u32(0);
                  uint32x4_t a2 = vdupq_n_u32(0), a3 = vdupq_n_u32(0);
                  const DATA32* restrict s = scol;

                  for (j = 0; j < diameter; j++, s += STEP)
                    {
                       const uint8x16_t px = vreinterpretq_u8_u32(vld1q_u32(s));
                       const uint16x8_t lo = vmovl_u8(vget_low_u8(px));
                       const uint16x8_t hi = vmovl_u8(vget_high_u8(px));
                       const uint16_t w = (uint16_t) weights[j];

                       a0 = vmlal_n_u16(a0, vget_low_u16(lo), w);
                       a1 = vmlal_n_u16(a1, vget_high_u16(lo), w);
                       a2 = vmlal_n_u16(a2, vget_low_u16(hi), w);
                       a3 = vmlal_n_u16(a3, vget_high_u16(hi), w);
                    }

                  a0 = vshlq_u32(a0, shift);
                  a1 = vshlq_u32(a1, shift);
                  a2 = vshlq_u32(a2, shift);
                  a3 = vshlq_u32(a3, shift);

                  vst1q_u32(dcol,
                            vreinterpretq_u32_u8(
                               vcombine_u8(vmovn_u16(vcombine_u16(vmovn_u32(a0),
                                                                  vmovn_u32(a1))),
                                           vmovn_u16(vcombine_u16(vmovn_u32(a2),
                                                                  vmovn_u32(a3))))));
               }
          }
        done_cols = c;
     }
#endif

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

             /* j is also the source position here, so the second condition is
              * what keeps a window wider than the line from running off the
              * end of it. It can only bite when len < 2 * radius + 1. */
             for (j = 0; (j <= k + radius) && (j < len); j++, s += STEP)
               {
                  const int weightidx = j + radius - k;
                  acc[ALPHA] += A_VAL(s) * weights[weightidx];
                  acc[RED]   += R_VAL(s) * weights[weightidx];
                  acc[GREEN] += G_VAL(s) * weights[weightidx];
                  acc[BLUE]  += B_VAL(s) * weights[weightidx];
                  divider += weights[weightidx];
               }
             if (!divider) goto error;
             A_VAL(dst) = acc[ALPHA] / divider;
             R_VAL(dst) = acc[RED]   / divider;
             G_VAL(dst) = acc[GREEN] / divider;
             B_VAL(dst) = acc[BLUE]  / divider;
          }

        // middle
        k = mid;
#ifdef BLUR_NEON
        if (STEP != 1)
          {
             /* already done by the vectorised column pass, for the columns it
              * covered; step over it */
             if (i > (loops - done_cols))
               {
                  src += roff * STEP;
                  dst += roff * STEP;
                  k = 0;
               }
          }
        else
          {
             /* four output pixels per iteration. Tap j contributes
              * src[k + j .. k + 3 + j], one unaligned 16 byte load, widened to
              * four 32 bit lanes per pixel and scaled by weights[j]. */
             for (; k >= 4; k -= 4, src += 4, dst += 4)
               {
                  uint32x4_t a0 = vdupq_n_u32(0), a1 = vdupq_n_u32(0);
                  uint32x4_t a2 = vdupq_n_u32(0), a3 = vdupq_n_u32(0);
                  const DATA32* restrict s = src;

                  for (j = 0; j < diameter; j++, s++)
                    {
                       const uint8x16_t px = vreinterpretq_u8_u32(vld1q_u32(s));
                       const uint16x8_t lo = vmovl_u8(vget_low_u8(px));
                       const uint16x8_t hi = vmovl_u8(vget_high_u8(px));
                       const uint16_t w = (uint16_t) weights[j];

                       a0 = vmlal_n_u16(a0, vget_low_u16(lo), w);
                       a1 = vmlal_n_u16(a1, vget_high_u16(lo), w);
                       a2 = vmlal_n_u16(a2, vget_low_u16(hi), w);
                       a3 = vmlal_n_u16(a3, vget_high_u16(hi), w);
                    }

                  a0 = vshlq_u32(a0, shift);
                  a1 = vshlq_u32(a1, shift);
                  a2 = vshlq_u32(a2, shift);
                  a3 = vshlq_u32(a3, shift);

                  vst1q_u32(dst,
                            vreinterpretq_u32_u8(
                               vcombine_u8(vmovn_u16(vcombine_u16(vmovn_u32(a0),
                                                                  vmovn_u32(a1))),
                                           vmovn_u16(vcombine_u16(vmovn_u32(a2),
                                                                  vmovn_u32(a3))))));
               }
          }
#endif
        for (; k > 0; k--, src += STEP, dst += STEP)
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

             /* src sits at roff + k, so roff + k + j is the source position;
              * as in the left ramp this only clamps when the window is wider
              * than the line */
             for (j = 0; (j < 2 * radius - k) && ((roff + k + j) < len); j++, s += STEP)
               {
                  acc[ALPHA] += A_VAL(s) * weights[j];
                  acc[RED]   += R_VAL(s) * weights[j];
                  acc[GREEN] += G_VAL(s) * weights[j];
                  acc[BLUE]  += B_VAL(s) * weights[j];
                  divider += weights[j];
               }
             if (!divider) goto error;
             A_VAL(dst) = acc[ALPHA] / divider;
             R_VAL(dst) = acc[RED]   / divider;
             G_VAL(dst) = acc[GREEN] / divider;
             B_VAL(dst) = acc[BLUE]  / divider;
          }

        dstdata += loopstep;
        srcdata += loopstep;
     }

   return;

error:
   CRI("Avoided division by 0.");
}

#undef FUNCTION_NAME
#undef STEP
#ifdef BLUR_NEON
# undef BLUR_NEON
#endif
