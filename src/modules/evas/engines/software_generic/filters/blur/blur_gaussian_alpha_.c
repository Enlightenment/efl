/* @file blur_gaussian_alpha_.c
 * Should define the functions:
 * - _gaussian_blur_horiz_alpha_step
 * - _gaussian_blur_vert_alpha_step
 *
 * Define BLUR_NEON as well to get the same kernel with its middle section
 * vectorised. See blur_gaussian_rgba_.c for the reasoning, including why there
 * is no separate NEON copy of this file. The only difference here is that an
 * alpha plane is one byte per pixel, so sixteen outputs fit in a register
 * instead of four.
 */

/* Datatypes and MIN macro */
#include "evas_filter_private.h"

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
   const int mid = len - (2 * radius);
   /* where the right ramp starts reading, i.e. how far the middle advanced */
   const int roff = (mid > 0) ? mid : 0;
   const DATA8* restrict s;
   const DATA8* restrict src;
   DATA8* restrict dst;
#ifdef BLUR_NEON
   const int32x4_t shift = vdupq_n_s32(-pow2_divider);
   int done_cols = 0;

   /* Vertical only: the middle of every column, sixteen columns at a time.
    * Each tap is one contiguous 16 byte load spanning sixteen columns of one
    * row, and the tap loop walks down by STEP. */
   if ((STEP != 1) && (mid > 0))
     {
        int c;

        for (c = 0; c + 16 <= loops; c += 16)
          {
             const DATA8* restrict scol = srcdata + c;
             DATA8* restrict dcol = dstdata + c + (size_t) left * STEP;

             for (k = 0; k < mid; k++, scol += STEP, dcol += STEP)
               {
                  uint32x4_t a0 = vdupq_n_u32(0), a1 = vdupq_n_u32(0);
                  uint32x4_t a2 = vdupq_n_u32(0), a3 = vdupq_n_u32(0);
                  const DATA8* restrict sc = scol;

                  for (j = 0; j < diameter; j++, sc += STEP)
                    {
                       const uint8x16_t px = vld1q_u8(sc);
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

                  vst1q_u8(dcol,
                           vcombine_u8(vmovn_u16(vcombine_u16(vmovn_u32(a0),
                                                              vmovn_u32(a1))),
                                       vmovn_u16(vcombine_u16(vmovn_u32(a2),
                                                              vmovn_u32(a3)))));
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
             acc = 0;
             divider = 0;
             s = src;
             /* j is also the source position here, so the second condition is
              * what keeps a window wider than the line from running off the
              * end of it. It can only bite when len < 2 * radius + 1. */
             for (j = 0; (j <= k + radius) && (j < len); j++, s += STEP)
               {
                  acc += (*s) * weights[j + radius - k];
                  divider += weights[j + radius - k];
               }
             if (!divider) goto error;
             *dst = acc / divider;
          }

        // middle
        k = radius;
#ifdef BLUR_NEON
        if (STEP != 1)
          {
             /* already done by the vectorised column pass, for the columns it
              * covered; step over it */
             if (i > (loops - done_cols))
               {
                  src += roff * STEP;
                  dst += roff * STEP;
                  k = len - radius;
               }
          }
        else
          {
             for (; k + 16 <= (len - radius); k += 16, src += 16, dst += 16)
               {
                  uint32x4_t a0 = vdupq_n_u32(0), a1 = vdupq_n_u32(0);
                  uint32x4_t a2 = vdupq_n_u32(0), a3 = vdupq_n_u32(0);

                  s = src;
                  for (j = 0; j < diameter; j++, s++)
                    {
                       const uint8x16_t px = vld1q_u8(s);
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

                  vst1q_u8(dst,
                           vcombine_u8(vmovn_u16(vcombine_u16(vmovn_u32(a0),
                                                              vmovn_u32(a1))),
                                       vmovn_u16(vcombine_u16(vmovn_u32(a2),
                                                              vmovn_u32(a3)))));
               }
          }
#endif
        for (; k < (len - radius); k++, src += STEP, dst += STEP)
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
             /* src sits at roff + k, so roff + k + j is the source position;
              * as in the left ramp this only clamps when the window is wider
              * than the line */
             for (j = 0; (j < 2 * radius - k) && ((roff + k + j) < len); j++, s += STEP)
               {
                  acc += (*s) * weights[j];
                  divider += weights[j];
               }
             if (!divider) goto error;
             *dst = acc / divider;
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
