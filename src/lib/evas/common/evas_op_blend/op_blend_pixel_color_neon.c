/* blend pixel x color --> dst */
#ifdef BUILD_NEON

#include <arm_neon.h>

/* Note: Optimisation is based on keeping _dest_ aligned: else it's a pair of
 * reads, then two writes, a miss on read is 'just' two reads */
static void
_op_blend_p_c_dp_neon(DATA32 * __restrict s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 * __restrict d, int l) {
#ifdef BUILD_NEON_INTRINSICS
   uint16x8_t ad0_16x8;
   uint16x8_t ad1_16x8;
   uint16x8_t sc0_16x8;
   uint16x8_t sc1_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t c_32x2;
   uint32x4_t ad_32x4;
   uint32x4_t alpha_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t d_32x4;
   uint32x4_t s_32x4;
   uint32x4_t sc_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad_8x16;
   uint8x16_t alpha_8x16;
   uint8x16_t d_8x16;
   uint8x16_t s_8x16;
   uint8x16_t sc_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad0_8x8;
   uint8x8_t ad1_8x8;
   uint8x8_t alpha0_8x8;
   uint8x8_t alpha1_8x8;
   uint8x8_t c_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;
   uint8x8_t sc0_8x8;
   uint8x8_t sc1_8x8;

   c_32x2 = vdup_n_u32(c);
   c_8x8 = vreinterpret_u8_u32(c_32x2);
   x255_16x8 = vdupq_n_u16(0xff);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);
   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~3);
   while (start < end)
   {

      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);

      d_32x4 = vld1q_u32(start);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      sc0_16x8 = vmull_u8(s0_8x8, c_8x8);
      sc1_16x8 = vmull_u8(s1_8x8, c_8x8);
      sc0_16x8 = vaddq_u16(sc0_16x8, x255_16x8);
      sc1_16x8 = vaddq_u16(sc1_16x8, x255_16x8);
      sc0_8x8 = vshrn_n_u16(sc0_16x8, 8);
      sc1_8x8 = vshrn_n_u16(sc1_16x8, 8);
      sc_8x16 = vcombine_u8(sc0_8x8, sc1_8x8);

      alpha_32x4 = vreinterpretq_u32_u8(sc_8x16);
      alpha_32x4 = vshrq_n_u32(alpha_32x4, 24);
      alpha_32x4 = vmulq_u32(x1_32x4, alpha_32x4);
      alpha_8x16 = vreinterpretq_u8_u32(alpha_32x4);
      alpha_8x16 = vsubq_u8(x0_8x16, alpha_8x16);
      alpha0_8x8 = vget_low_u8(alpha_8x16);
      alpha1_8x8 = vget_high_u8(alpha_8x16);

      ad0_16x8 = vmull_u8(alpha0_8x8, d0_8x8);
      ad1_16x8 = vmull_u8(alpha1_8x8, d1_8x8);
      ad0_8x8 = vshrn_n_u16(ad0_16x8,8);
      ad1_8x8 = vshrn_n_u16(ad1_16x8,8);
      ad_8x16 = vcombine_u8(ad0_8x8, ad1_8x8);
      ad_32x4 = vreinterpretq_u32_u8(ad_8x16);

      alpha_32x4 = vreinterpretq_u32_u8(alpha_8x16);
      cond_32x4 = vceqq_u32(alpha_32x4, x0_32x4);
      ad_32x4 = vbslq_u32(cond_32x4, d_32x4 , ad_32x4);

      sc_32x4 = vreinterpretq_u32_u8(sc_8x16);
      d_32x4 = vaddq_u32(sc_32x4, ad_32x4);

      vst1q_u32(start, d_32x4);

      s+=4;
      start+=4;
   }
   end += (size & 3);
   while (start <  end)
   {
      DATA32 sc = MUL4_SYM(c, *s);
      DATA32 alpha = 256 - (sc >> 24);
      *start = sc + MUL_256(alpha, *start);
      start++;
      s++;
   }
#else
#define AP	"blend_p_c_dp_"
   asm volatile (
      ".fpu neon\n\t"
      "vdup.u32  d0, %[c]\n\t"          // Load 'c'
      "vmov.u16  q1, $0x00ff\n\t"       // round_mask
      "vmov.u8   q2, #0\n\t"            // zero register
      "sub       %[tmp], %[e], #16\n\t"
      "cmp       %[d], %[tmp]\n\t"
      "bhi       "AP"skipquad\n\t"
    AP"quadloop:"
      "vld1.32   {d6, d7}, [%[s]]!\n\t" // Load 's'
      "vld1.32   {d8, d9}, [%[d]]\n\t"  // Load 'd'
      "vmull.u8  q5, d6, d0\n\t"        // s * c
      "vmull.u8  q6, d7, d0\n\t"
      "vadd.u16  q5, q5, q1\n\t"        // rounding
      "vadd.u16  q6, q6, q1\n\t"
      "vshrn.u16 d10, q5, #8\n\t"       // narrowing
      "vshrn.u16 d11, q6, #8\n\t"       // sc in q5
      "vsub.u8   q6, q2, q5\n\t"
      "vmov      q7, q6\n\t"
      "vtrn.u8   q7, q6\n\t"
      "vmov      q7, q6\n\t"
      "vtrn.u16  q7, q6\n\t"            // q6 - alpha
      "vmull.u8  q7, d8, d12\n\t"
      "vmull.u8  q8, d9, d13\n\t"
      "vshrn.u16 d14, q7, #8\n\t"
      "vshrn.u16 d15, q8, #8\n\t"       // q7 - d * alpha
      "vceq.i32  q6, q6, #0\n\t"        // if alpha = 0x100
      "vbsl      q6, q4, q7\n\t"        // just copy d[i]
      "vadd.u32  q4, q5, q6\n\t"
      "vst1.u32  {d8, d9}, [%[d]]!\n\t"
      "cmp       %[d], %[tmp]\n\t"
      "bls       "AP"quadloop\n\t"
    AP"skipquad:"
      "sub       %[tmp], %[e], #8\n\t"
      "cmp       %[d], %[tmp]\n\t"
      "bhi       "AP"skipdouble\n\t"
    AP"doubleloop:"
      "vld1.32   d6, [%[s]]!\n\t"
      "vld1.32   d7, [%[d]]\n\t"
      "vmull.u8  q4, d6, d0\n\t"
      "vadd.u16  q4, q4, q1\n\t"
      "vshrn.u16 d8, q4, #8\n\t"
      "vsub.u8   d9, d4, d8\n\t"
      "vmov      d10, d9\n\t"
      "vtrn.u8   d10, d9\n\t"
      "vmov      d10, d9\n\t"
      "vtrn.u16  d10, d9\n\t"           // d9 - alpha
      "vmull.u8  q5, d7, d9\n\t"
      "vshrn.u16 d1, q5, #8\n\t"
      "vceq.i32  d9, d9, #0\n\t"
      "vbsl      d9, d7, d1\n\t"        // d7 - d[i], d1 - d[i] * alpha
      "vadd.u32  d7, d8, d9\n\t"
      "vst1.u32  d7, [%[d]]!\n\t"
      "cmp       %[d], %[tmp]\n\t"
      "bls       "AP"doubleloop\n\t"
    AP"skipdouble:"
      "cmp       %[d], %[e]\n\t"
      "beq       "AP"done\n\t"
    AP"singleloop:"
      "vld1.32   d6[0], [%[s]]!\n\t"
      "vld1.32   d7[0], [%[d]]\n\t"
      "vmull.u8  q4, d6, d0\n\t"
      "vadd.u16  q4, q4, q1\n\t"
      "vshrn.u16 d8, q4, #8\n\t"
      "vsub.u8   d9, d4, d8\n\t"
      "vmov      d10, d9\n\t"
      "vtrn.u8   d10, d9\n\t"
      "vmov      d10, d9\n\t"
      "vtrn.u16  d10, d9\n\t"           // d9 - alpha
      "vmull.u8  q5, d7, d9\n\t"
      "vshrn.u16 d1, q5, #8\n\t"
      "vceq.i32  d9, d9, #0\n\t"
      "vbsl      d9, d7, d1\n\t"        // d7 - d[i], d1 - d[i] * alpha
      "vadd.u32  d7, d8, d9\n\t"
      "vst1.u32  d7[0], [%[d]]!\n\t"
      "cmp       %[d], %[e]\n\t"
      "blt       "AP"singleloop\n\t"
    AP"done:"
      : // No output
      : [s] "r" (s), [d] "r" (d), [c] "r" (c), [e] "r" (d + l), [tmp] "r" (12)
      : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "memory"
   );
#undef AP
#endif
}


static void
_op_blend_pan_c_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t ad0_16x8;
   uint16x8_t ad1_16x8;
   uint16x8_t sc0_16x8;
   uint16x8_t sc1_16x8;
   uint16x8_t x255_16x8;
   uint32x4_t ad_32x4;
   uint32x4_t c_32x4;
   uint32x4_t d_32x4;
   uint32x4_t mask_32x4;
   uint32x4_t s_32x4;
   uint32x4_t sc_32x4;
   uint8x16_t ad_8x16;
   uint8x16_t c_8x16;
   uint8x16_t d_8x16;
   uint8x16_t mask_8x16;
   uint8x16_t s_8x16;
   uint8x16_t sc_8x16;
   uint8x8_t a_8x8;
   uint8x8_t ad0_8x8;
   uint8x8_t ad1_8x8;
   uint8x8_t c_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;
   uint8x8_t sc0_8x8;
   uint8x8_t sc1_8x8;

   // alpha can only be 0 if color is 0x0. In that case we can just return.
   // Otherwise we can assume alpha != 0. This allows more optimization in
   // NEON code.

   if(!c)
      return;

   unsigned char a;
   a = ~(c >> 24) + 1; // 256 - (c >> 24)

   a_8x8 = vdup_n_u8(a);
   c_32x4 = vdupq_n_u32(c);
   c_8x16 = vreinterpretq_u8_u32(c_32x4);
   c_8x8 = vget_low_u8(c_8x16);
   x255_16x8 = vdupq_n_u16(0xff);
   mask_32x4 = vdupq_n_u32(0xff000000);
   mask_8x16 = vreinterpretq_u8_u32(mask_32x4);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from d
      d_32x4 = vld1q_u32(d);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      // multiply MUL_256(a, *d)
      ad0_16x8 = vmull_u8(a_8x8, d0_8x8);
      ad1_16x8 = vmull_u8(a_8x8, d1_8x8);
      ad0_8x8 = vshrn_n_u16(ad0_16x8,8);
      ad1_8x8 = vshrn_n_u16(ad1_16x8,8);
      ad_8x16 = vcombine_u8(ad0_8x8, ad1_8x8);
      ad_32x4 = vreinterpretq_u32_u8(ad_8x16);

      // load 4 elements from s
      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);
      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      // multiply MUL_SYM(c, *s);
      sc0_16x8 = vmull_u8(s0_8x8, c_8x8);
      sc1_16x8 = vmull_u8(s1_8x8, c_8x8);
      sc0_16x8 = vaddq_u16(sc0_16x8, x255_16x8);
      sc1_16x8 = vaddq_u16(sc1_16x8, x255_16x8);
      sc0_8x8 = vshrn_n_u16(sc0_16x8, 8);
      sc1_8x8 = vshrn_n_u16(sc1_16x8, 8);
      sc_8x16 = vcombine_u8(sc0_8x8, sc1_8x8);

      // select alpha channel from c
      sc_8x16 = vbslq_u8(mask_8x16, c_8x16, sc_8x16);
      sc_32x4 = vreinterpretq_u32_u8(sc_8x16);

      // add up everything
      d_32x4 = vaddq_u32(sc_32x4, ad_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
      s+=4;
   }

   end += (l & 3);
   while (d < end)
   {
      *d = ((c & 0xff000000) + MUL3_SYM(c, *s)) + MUL_256(a, *d);
      d++;
      s++;
   }

}

static void
_op_blend_p_can_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t ad0_16x8;
   uint16x8_t ad1_16x8;
   uint16x8_t sc0_16x8;
   uint16x8_t sc1_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t c_32x2;
   uint32x4_t ad_32x4;
   uint32x4_t alpha_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t d_32x4;
   uint32x4_t mask_32x4;
   uint32x4_t s_32x4;
   uint32x4_t sc_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad_8x16;
   uint8x16_t alpha_8x16;
   uint8x16_t d_8x16;
   uint8x16_t mask_8x16;
   uint8x16_t s_8x16;
   uint8x16_t sc_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad0_8x8;
   uint8x8_t ad1_8x8;
   uint8x8_t alpha0_8x8;
   uint8x8_t alpha1_8x8;
   uint8x8_t c_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;
   uint8x8_t sc0_8x8;
   uint8x8_t sc1_8x8;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);
   mask_32x4 = vdupq_n_u32(0xff000000);
   mask_8x16 = vreinterpretq_u8_u32(mask_32x4);
   c_32x2 = vdup_n_u32(c);
   c_8x8 = vreinterpret_u8_u32(c_32x2);
   x255_16x8 = vdupq_n_u16(0xff);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from s
      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);
      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      // load 4 elements from d
      d_32x4 = vld1q_u32(d);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      // calculate alpha = 256 - (*s >> 24)
      alpha_32x4 = vshrq_n_u32(s_32x4, 24);
      alpha_32x4 = vmulq_u32(x1_32x4, alpha_32x4);
      alpha_8x16 = vreinterpretq_u8_u32(alpha_32x4);
      alpha_8x16 = vsubq_u8(x0_8x16, alpha_8x16);
      alpha0_8x8 = vget_low_u8(alpha_8x16);
      alpha1_8x8 = vget_high_u8(alpha_8x16);

      // multiply MUL_SYM(c, *s);
      sc0_16x8 = vmull_u8(s0_8x8, c_8x8);
      sc1_16x8 = vmull_u8(s1_8x8, c_8x8);
      sc0_16x8 = vaddq_u16(sc0_16x8, x255_16x8);
      sc1_16x8 = vaddq_u16(sc1_16x8, x255_16x8);
      sc0_8x8 = vshrn_n_u16(sc0_16x8, 8);
      sc1_8x8 = vshrn_n_u16(sc1_16x8, 8);
      sc_8x16 = vcombine_u8(sc0_8x8, sc1_8x8);

      // select alpha channel from *s
      sc_8x16 = vbslq_u8(mask_8x16, s_8x16, sc_8x16);
      sc_32x4 = vreinterpretq_u32_u8(sc_8x16);

      // multiply MUL_256(a, *d)
      ad0_16x8 = vmull_u8(alpha0_8x8, d0_8x8);
      ad1_16x8 = vmull_u8(alpha1_8x8, d1_8x8);
      ad0_8x8 = vshrn_n_u16(ad0_16x8,8);
      ad1_8x8 = vshrn_n_u16(ad1_16x8,8);
      ad_8x16 = vcombine_u8(ad0_8x8, ad1_8x8);
      ad_32x4 = vreinterpretq_u32_u8(ad_8x16);

      // select d if alpha is 0
      cond_32x4 = vceqq_u32(alpha_32x4, x0_32x4);
      ad_32x4 = vbslq_u32(cond_32x4, d_32x4, ad_32x4);

      // add up everything
      d_32x4 = vaddq_u32(sc_32x4, ad_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
      s+=4;
   }

   end += (l & 3);
   int alpha;
   while (d < end)
   {
      alpha = 256 - (*s >> 24);
      *d = ((*s & 0xff000000) + MUL3_SYM(c, *s)) + MUL_256(alpha, *d);
      d++;
      s++;
   }
}

static void
_op_blend_pan_can_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t sc00_16x8;
   uint16x8_t sc01_16x8;
   uint16x8_t sc10_16x8;
   uint16x8_t sc11_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t c_32x2;
   uint32x4_t d0_32x4;
   uint32x4_t d1_32x4;
   uint32x4_t mask_32x4;
   uint32x4_t s0_32x4;
   uint32x4_t s1_32x4;
   uint32x4_t sc0_32x4;
   uint32x4_t sc1_32x4;
   uint8x16_t s0_8x16;
   uint8x16_t s1_8x16;
   uint8x16_t sc0_8x16;
   uint8x16_t sc1_8x16;
   uint8x8_t c_8x8;
   uint8x8_t s00_8x8;
   uint8x8_t s01_8x8;
   uint8x8_t s10_8x8;
   uint8x8_t s11_8x8;
   uint8x8_t sc00_8x8;
   uint8x8_t sc01_8x8;
   uint8x8_t sc10_8x8;
   uint8x8_t sc11_8x8;

   mask_32x4 = vdupq_n_u32(0xff000000);
   x255_16x8 = vdupq_n_u16(0xff);
   c_32x2 = vdup_n_u32(c);
   c_8x8 = vreinterpret_u8_u32(c_32x2);

   DATA32 *end = d + (l & ~7);
   while (d < end)
   {
      // load 8 elements from s
      s0_32x4 = vld1q_u32(s);
      s0_8x16 = vreinterpretq_u8_u32(s0_32x4);
      s00_8x8 = vget_low_u8(s0_8x16);
      s01_8x8 = vget_high_u8(s0_8x16);
      s1_32x4 = vld1q_u32(s+4);
      s1_8x16 = vreinterpretq_u8_u32(s1_32x4);
      s10_8x8 = vget_low_u8(s1_8x16);
      s11_8x8 = vget_high_u8(s1_8x16);

      // multiply MUL_SYM(c, *s);
      sc00_16x8 = vmull_u8(s00_8x8, c_8x8);
      sc01_16x8 = vmull_u8(s01_8x8, c_8x8);
      sc10_16x8 = vmull_u8(s10_8x8, c_8x8);
      sc11_16x8 = vmull_u8(s11_8x8, c_8x8);
      sc00_16x8 = vaddq_u16(sc00_16x8, x255_16x8);
      sc01_16x8 = vaddq_u16(sc01_16x8, x255_16x8);
      sc10_16x8 = vaddq_u16(sc10_16x8, x255_16x8);
      sc11_16x8 = vaddq_u16(sc11_16x8, x255_16x8);
      sc00_8x8 = vshrn_n_u16(sc00_16x8, 8);
      sc01_8x8 = vshrn_n_u16(sc01_16x8, 8);
      sc10_8x8 = vshrn_n_u16(sc10_16x8, 8);
      sc11_8x8 = vshrn_n_u16(sc11_16x8, 8);
      sc0_8x16 = vcombine_u8(sc00_8x8, sc01_8x8);
      sc1_8x16 = vcombine_u8(sc10_8x8, sc11_8x8);

      // add alpha channel
      sc0_32x4 = vreinterpretq_u32_u8(sc0_8x16);
      sc1_32x4 = vreinterpretq_u32_u8(sc1_8x16);
      d0_32x4 = vorrq_u32(sc0_32x4, mask_32x4);
      d1_32x4 = vorrq_u32(sc1_32x4, mask_32x4);

      // save result
      vst1q_u32(d, d0_32x4);
      vst1q_u32(d+4, d1_32x4);

      d+=8;
      s+=8;
   }

   end += (l & 7);
   while (d < end)
   {
      *d++ = 0xff000000 + MUL3_SYM(c, *s);
      s++;
   }
}

static void
_op_blend_p_caa_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t ad0_16x8;
   uint16x8_t ad1_16x8;
   uint16x8_t cs0_16x8;
   uint16x8_t cs1_16x8;
   uint32x4_t ad_32x4;
   uint32x4_t alpha_32x4;
   uint32x4_t c_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t cs_32x4;
   uint32x4_t d_32x4;
   uint32x4_t s_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad_8x16;
   uint8x16_t alpha_8x16;
   uint8x16_t c_8x16;
   uint8x16_t cs_8x16;
   uint8x16_t d_8x16;
   uint8x16_t s_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad0_8x8;
   uint8x8_t ad1_8x8;
   uint8x8_t alpha0_8x8;
   uint8x8_t alpha1_8x8;
   uint8x8_t c_8x8;
   uint8x8_t cs0_8x8;
   uint8x8_t cs1_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;

   int temp = (1 + c) & 0xff;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   c_32x4 = vdupq_n_u32(temp);
   c_32x4 = vmulq_u32(x1_32x4, c_32x4);
   c_8x16 = vreinterpretq_u8_u32(c_32x4);
   c_8x8 = vget_low_u8(c_8x16);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from s
      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);
      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      // multiply MUL_256(c, *s)
      cs0_16x8 = vmull_u8(c_8x8, s0_8x8);
      cs1_16x8 = vmull_u8(c_8x8, s1_8x8);
      cs0_8x8 = vshrn_n_u16(cs0_16x8,8);
      cs1_8x8 = vshrn_n_u16(cs1_16x8,8);
      cs_8x16 = vcombine_u8(cs0_8x8, cs1_8x8);
      cs_32x4 = vreinterpretq_u32_u8(cs_8x16);

      // select s if c is 0
      cond_32x4 = vceqq_u32(c_32x4, x0_32x4);
      cs_32x4 = vbslq_u32(cond_32x4, s_32x4 , cs_32x4);

      // calculate alpha = 256 - (*s >> 24)
      alpha_32x4 = vshrq_n_u32(cs_32x4, 24);
      alpha_32x4 = vmulq_u32(x1_32x4, alpha_32x4);
      alpha_8x16 = vreinterpretq_u8_u32(alpha_32x4);
      alpha_8x16 = vsubq_u8(x0_8x16, alpha_8x16);
      alpha0_8x8 = vget_low_u8(alpha_8x16);
      alpha1_8x8 = vget_high_u8(alpha_8x16);

      // load 4 elements from d
      d_32x4 = vld1q_u32(d);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      // multiply MUL_256(a, *d)
      ad0_16x8 = vmull_u8(alpha0_8x8, d0_8x8);
      ad1_16x8 = vmull_u8(alpha1_8x8, d1_8x8);
      ad0_8x8 = vshrn_n_u16(ad0_16x8,8);
      ad1_8x8 = vshrn_n_u16(ad1_16x8,8);
      ad_8x16 = vcombine_u8(ad0_8x8, ad1_8x8);
      ad_32x4 = vreinterpretq_u32_u8(ad_8x16);

      // select d if alpha is 0
      alpha_32x4 = vreinterpretq_u32_u8(alpha_8x16);
      cond_32x4 = vceqq_u32(alpha_32x4, x0_32x4);
      ad_32x4 = vbslq_u32(cond_32x4, d_32x4 , ad_32x4);

      // add up everything
      d_32x4 = vaddq_u32(cs_32x4, ad_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
      s+=4;
   }

   end += (l & 3);
   int alpha;
   c = 1 + (c & 0xff);
   while (d < end)
   {
      DATA32 sc = MUL_256(c, *s);
      alpha = 256 - (sc >> 24);
      *d = sc + MUL_256(alpha, *d);
      d++;
      s++;
   }

}

static void
_op_blend_pan_caa_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   int16x8_t c_i16x8;
   int16x8_t d0_i16x8;
   int16x8_t d1_i16x8;
   int16x8_t ds0_i16x8;
   int16x8_t ds1_i16x8;
   int16x8_t s0_i16x8;
   int16x8_t s1_i16x8;
   int8x16_t ds_i8x16;
   int8x8_t ds0_i8x8;
   int8x8_t ds1_i8x8;
   uint16x8_t c_16x8;
   uint16x8_t d0_16x8;
   uint16x8_t d1_16x8;
   uint16x8_t s0_16x8;
   uint16x8_t s1_16x8;
   uint32x4_t d_32x4;
   uint32x4_t ds_32x4;
   uint32x4_t s_32x4;
   uint8x16_t d_8x16;
   uint8x16_t s_8x16;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;

   c = 1 + (c & 0xff);

   c_16x8 = vdupq_n_u16(c);
   c_i16x8 = vreinterpretq_s16_u16(c_16x8);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from d
      d_32x4 = vld1q_u32(d);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      // spread d so that each channel occupies 16 bit
      d0_16x8 = vmovl_u8(d0_8x8);
      d1_16x8 = vmovl_u8(d1_8x8);
      d0_i16x8 = vreinterpretq_s16_u16(d0_16x8);
      d1_i16x8 = vreinterpretq_s16_u16(d1_16x8);

      // load 4 elements from s
      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);
      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      // spread s so that each channel occupies 16 bit
      s0_16x8 = vmovl_u8(s0_8x8);
      s1_16x8 = vmovl_u8(s1_8x8);
      s0_i16x8 = vreinterpretq_s16_u16(s0_16x8);
      s1_i16x8 = vreinterpretq_s16_u16(s1_16x8);

      // interpolate
      ds0_i16x8 = vsubq_s16(s0_i16x8, d0_i16x8);
      ds1_i16x8 = vsubq_s16(s1_i16x8, d1_i16x8);
      ds0_i16x8 = vmulq_s16(ds0_i16x8, c_i16x8);
      ds1_i16x8 = vmulq_s16(ds1_i16x8, c_i16x8);
      ds0_i16x8 = vshrq_n_s16(ds0_i16x8, 8);
      ds1_i16x8 = vshrq_n_s16(ds1_i16x8, 8);
      ds0_i16x8 = vaddq_s16(ds0_i16x8, d0_i16x8);
      ds1_i16x8 = vaddq_s16(ds1_i16x8, d1_i16x8);
      ds0_i8x8 = vmovn_s16(ds0_i16x8);
      ds1_i8x8 = vmovn_s16(ds1_i16x8);

      // save result
      ds_i8x16 = vcombine_s8(ds0_i8x8, ds1_i8x8);
      ds_32x4 = vreinterpretq_u32_s8(ds_i8x16);
      vst1q_u32(d, ds_32x4);

      d+=4;
      s+=4;
   }

   end += (l & 3);
   while (d < end)
   {
      *d = INTERP_256(c, *s, *d);
      d++;
      s++;
   }
}

#define _op_blend_pas_c_dp_neon _op_blend_p_c_dp_neon
#define _op_blend_pas_can_dp_neon _op_blend_p_c_dp_neon
#define _op_blend_pas_caa_dp_neon _op_blend_p_c_dp_neon

#define _op_blend_p_c_dpan_neon _op_blend_p_c_dp_neon
#define _op_blend_pas_c_dpan_neon _op_blend_pas_c_dp_neon
#define _op_blend_pan_c_dpan_neon _op_blend_pan_c_dp_neon
#define _op_blend_p_can_dpan_neon _op_blend_p_can_dp_neon
#define _op_blend_pas_can_dpan_neon _op_blend_pas_can_dp_neon
#define _op_blend_pan_can_dpan_neon _op_blend_pan_can_dp_neon
#define _op_blend_p_caa_dpan_neon _op_blend_p_caa_dp_neon
#define _op_blend_pas_caa_dpan_neon _op_blend_pas_caa_dp_neon
#define _op_blend_pan_caa_dpan_neon _op_blend_pan_caa_dp_neon


static void
init_blend_pixel_color_span_funcs_neon(void)
{
   op_blend_span_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_blend_p_c_dp_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_blend_pas_c_dp_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_blend_pan_c_dp_neon;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_p_can_dp_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pas_can_dp_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pan_can_dp_neon;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_p_caa_dp_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pas_caa_dp_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pan_caa_dp_neon;

   op_blend_span_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_p_c_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pas_c_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pan_c_dpan_neon;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_p_can_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pas_can_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pan_can_dpan_neon;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_p_caa_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pas_caa_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pan_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_p_c_dp_neon(DATA32 s, DATA8 m EINA_UNUSED, DATA32 c, DATA32 *d) {
   s = MUL4_SYM(c, s);
   c = 256 - (s >> 24);
   *d = s + MUL_256(c, *d);
}

#define _op_blend_pt_pas_c_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_c_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_can_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_can_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_can_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_caa_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_caa_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_caa_dp_neon _op_blend_pt_p_c_dp_neon

#define _op_blend_pt_p_c_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_c_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_c_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_can_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_can_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_can_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_caa_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_caa_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_caa_dpan_neon _op_blend_pt_p_c_dp_neon

static void
init_blend_pixel_color_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_p_c_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_pas_c_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_pan_c_dp_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pt_p_can_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pt_pas_can_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pt_pan_can_dp_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_p_caa_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_pas_caa_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_pan_caa_dp_neon;

   op_blend_pt_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_p_c_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_pas_c_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_pan_c_dpan_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pt_p_can_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pt_pas_can_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pt_pan_can_dpan_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_p_caa_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_pas_caa_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_pan_caa_dpan_neon;
}
#endif

/*-----*/

/* blend_rel pixel x color -> dst */

#ifdef BUILD_NEON

static void
_op_blend_rel_p_c_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t ad0_16x8;
   uint16x8_t ad1_16x8;
   uint16x8_t dsc0_16x8;
   uint16x8_t dsc1_16x8;
   uint16x8_t sc0_16x8;
   uint16x8_t sc1_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t c_32x2;
   uint32x4_t ad_32x4;
   uint32x4_t alpha_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t d_32x4;
   uint32x4_t dsc_32x4;
   uint32x4_t s_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad_8x16;
   uint8x16_t alpha_8x16;
   uint8x16_t d_8x16;
   uint8x16_t dsc_8x16;
   uint8x16_t s_8x16;
   uint8x16_t sc_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad0_8x8;
   uint8x8_t ad1_8x8;
   uint8x8_t alpha0_8x8;
   uint8x8_t alpha1_8x8;
   uint8x8_t c_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t dsc0_8x8;
   uint8x8_t dsc1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;
   uint8x8_t sc0_8x8;
   uint8x8_t sc1_8x8;

   c_32x2 = vdup_n_u32(c);
   c_8x8 = vreinterpret_u8_u32(c_32x2);
   x255_16x8 = vdupq_n_u16(0xff);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);
   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from s
      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);
      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      // load 4 elements from d
      d_32x4 = vld1q_u32(d);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      // multiply MUL4_SYM(c, *s);
      sc0_16x8 = vmull_u8(s0_8x8, c_8x8);
      sc1_16x8 = vmull_u8(s1_8x8, c_8x8);
      sc0_16x8 = vaddq_u16(sc0_16x8, x255_16x8);
      sc1_16x8 = vaddq_u16(sc1_16x8, x255_16x8);
      sc0_8x8 = vshrn_n_u16(sc0_16x8, 8);
      sc1_8x8 = vshrn_n_u16(sc1_16x8, 8);
      sc_8x16 = vcombine_u8(sc0_8x8, sc1_8x8);

      // calculate alpha = 256 - (sc >> 24)
      alpha_32x4 = vreinterpretq_u32_u8(sc_8x16);
      alpha_32x4 = vshrq_n_u32(alpha_32x4, 24);
      alpha_32x4 = vmulq_u32(x1_32x4, alpha_32x4);
      alpha_8x16 = vreinterpretq_u8_u32(alpha_32x4);
      alpha_8x16 = vsubq_u8(x0_8x16, alpha_8x16);
      alpha0_8x8 = vget_low_u8(alpha_8x16);
      alpha1_8x8 = vget_high_u8(alpha_8x16);

      // multiply MUL_256(alpha, *d);
      ad0_16x8 = vmull_u8(alpha0_8x8, d0_8x8);
      ad1_16x8 = vmull_u8(alpha1_8x8, d1_8x8);
      ad0_8x8 = vshrn_n_u16(ad0_16x8,8);
      ad1_8x8 = vshrn_n_u16(ad1_16x8,8);
      ad_8x16 = vcombine_u8(ad0_8x8, ad1_8x8);
      ad_32x4 = vreinterpretq_u32_u8(ad_8x16);

      // select d when alpha is 0
      alpha_32x4 = vreinterpretq_u32_u8(alpha_8x16);
      cond_32x4 = vceqq_u32(alpha_32x4, x0_32x4);
      ad_32x4 = vbslq_u32(cond_32x4, d_32x4 , ad_32x4);

      // shift (*d >> 24)
      dsc_32x4 = vshrq_n_u32(d_32x4, 24);
      dsc_32x4 = vmulq_u32(x1_32x4, dsc_32x4);
      dsc_8x16 = vreinterpretq_u8_u32(dsc_32x4);
      dsc0_8x8 = vget_low_u8(dsc_8x16);
      dsc1_8x8 = vget_high_u8(dsc_8x16);

      // multiply MUL_256(*d >> 24, sc);
      dsc0_16x8 = vmull_u8(dsc0_8x8, sc0_8x8);
      dsc1_16x8 = vmull_u8(dsc1_8x8, sc1_8x8);
      dsc0_16x8 = vaddq_u16(dsc0_16x8, x255_16x8);
      dsc1_16x8 = vaddq_u16(dsc1_16x8, x255_16x8);
      dsc0_8x8 = vshrn_n_u16(dsc0_16x8, 8);
      dsc1_8x8 = vshrn_n_u16(dsc1_16x8, 8);
      dsc_8x16 = vcombine_u8(dsc0_8x8, dsc1_8x8);

      // add up everything
      dsc_32x4 = vreinterpretq_u32_u8(dsc_8x16);
      d_32x4 = vaddq_u32(dsc_32x4, ad_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
      s+=4;
   }

   end += (l & 3);
   int alpha;
   while (d < end)
   {
      DATA32 sc = MUL4_SYM(c, *s);
      alpha = 256 - (sc >> 24);
      *d = MUL_SYM(*d >> 24, sc) + MUL_256(alpha, *d);
      d++;
      s++;
   }
}

#define _op_blend_rel_pas_c_dp_neon _op_blend_rel_p_c_dp_neon
#define _op_blend_rel_pan_c_dp_neon _op_blend_rel_p_c_dp_neon
#define _op_blend_rel_p_can_dp_neon _op_blend_rel_p_c_dp_neon
#define _op_blend_rel_pas_can_dp_neon _op_blend_rel_p_c_dp_neon
#define _op_blend_rel_pan_can_dp_neon _op_blend_rel_p_c_dp_neon
#define _op_blend_rel_p_caa_dp_neon _op_blend_rel_p_c_dp_neon
#define _op_blend_rel_pas_caa_dp_neon _op_blend_rel_p_c_dp_neon
#define _op_blend_rel_pan_caa_dp_neon _op_blend_rel_p_c_dp_neon

#define _op_blend_rel_p_c_dpan_neon _op_blend_p_c_dpan_neon
#define _op_blend_rel_pas_c_dpan_neon _op_blend_pas_c_dpan_neon
#define _op_blend_rel_pan_c_dpan_neon _op_blend_pan_c_dpan_neon
#define _op_blend_rel_p_can_dpan_neon _op_blend_p_can_dpan_neon
#define _op_blend_rel_pas_can_dpan_neon _op_blend_pas_can_dpan_neon
#define _op_blend_rel_pan_can_dpan_neon _op_blend_pan_can_dpan_neon
#define _op_blend_rel_p_caa_dpan_neon _op_blend_p_caa_dpan_neon
#define _op_blend_rel_pas_caa_dpan_neon _op_blend_pas_caa_dpan_neon
#define _op_blend_rel_pan_caa_dpan_neon _op_blend_pan_caa_dpan_neon

static void
init_blend_rel_pixel_color_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_blend_rel_p_c_dp_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_blend_rel_pas_c_dp_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_blend_rel_pan_c_dp_neon;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_rel_p_can_dp_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_rel_pas_can_dp_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_rel_pan_can_dp_neon;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_rel_p_caa_dp_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_rel_pas_caa_dp_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_rel_pan_caa_dp_neon;

   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_p_c_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pas_c_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pan_c_dpan_neon;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_p_can_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pas_can_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pan_can_dpan_neon;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_p_caa_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pas_caa_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pan_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON

#define _op_blend_rel_pt_p_c_dpan_neon _op_blend_pt_p_c_dpan_neon
#define _op_blend_rel_pt_pas_c_dpan_neon _op_blend_pt_pas_c_dpan_neon
#define _op_blend_rel_pt_pan_c_dpan_neon _op_blend_pt_pan_c_dpan_neon
#define _op_blend_rel_pt_p_can_dpan_neon _op_blend_pt_p_can_dpan_neon
#define _op_blend_rel_pt_pas_can_dpan_neon _op_blend_pt_pas_can_dpan_neon
#define _op_blend_rel_pt_pan_can_dpan_neon _op_blend_pt_pan_can_dpan_neon
#define _op_blend_rel_pt_p_caa_dpan_neon _op_blend_pt_p_caa_dpan_neon
#define _op_blend_rel_pt_pas_caa_dpan_neon _op_blend_pt_pas_caa_dpan_neon
#define _op_blend_rel_pt_pan_caa_dpan_neon _op_blend_pt_pan_caa_dpan_neon

static void
init_blend_rel_pixel_color_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_c_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_c_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_c_dpan_neon;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_can_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_can_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_can_dpan_neon;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_caa_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_caa_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_caa_dpan_neon;
}
#endif
