#ifdef BUILD_NEON
#include <arm_neon.h>
#endif
/* blend pixel x mask --> dst */

#ifdef BUILD_NEON
static void
_op_blend_pas_mas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c EINA_UNUSED, DATA32 *d, int l) {
   uint16x8_t m_16x8;
   uint16x8_t ms0_16x8;
   uint16x8_t ms1_16x8;
   uint16x8_t temp0_16x8;
   uint16x8_t temp1_16x8;
   uint16x8_t x255_16x8;
   uint32_t m_32;
   uint32x2_t m_32x2;
   uint32x4_t a_32x4;
   uint32x4_t ad_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t d_32x4;
   uint32x4_t m_32x4;
   uint32x4_t ms_32x4;
   uint32x4_t s_32x4;
   uint32x4_t temp_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t a_8x16;
   uint8x16_t d_8x16;
   uint8x16_t m_8x16;
   uint8x16_t ms_8x16;
   uint8x16_t s_8x16;
   uint8x16_t temp_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t a0_8x8;
   uint8x8_t a1_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t m0_8x8;
   uint8x8_t m1_8x8;
   uint8x8_t m_8x8;
   uint8x8_t ms0_8x8;
   uint8x8_t ms1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;
   uint8x8_t temp0_8x8;
   uint8x8_t temp1_8x8;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x255_16x8 = vdupq_n_u16(0xff);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      unsigned int k = *((unsigned int *)m);
      // shortcut if *m==0
      if (k == 0)
      {
         m+=4;
         d+=4;
         s+=4;
         continue;
      }
      // shortcut if *m==0xff
      if (~k == 0)
      {
         // load 4 elements from s
         s_32x4 = vld1q_u32(s);
         s_8x16 = vreinterpretq_u8_u32(s_32x4);

         // load 4 elements from d
         d_32x4 = vld1q_u32(d);
         d_8x16 = vreinterpretq_u8_u32(d_32x4);
         d0_8x8 = vget_low_u8(d_8x16);
         d1_8x8 = vget_high_u8(d_8x16);

         // substract 256 - *s
         a_8x16 = vsubq_u8(x0_8x16, s_8x16);
         a_32x4 = vreinterpretq_u32_u8(a_8x16);

         // shift alpha>>24 and place it into every 8bit element
         a_32x4 = vshrq_n_u32(a_32x4, 24);
         a_32x4 = vmulq_u32(a_32x4, x1_32x4);
         a_8x16 = vreinterpretq_u8_u32(a_32x4);
         a0_8x8 = vget_low_u8(a_8x16);
         a1_8x8 = vget_high_u8(a_8x16);

         // multiply MUL_256(a, *d)
         temp0_16x8 = vmull_u8(a0_8x8, d0_8x8);
         temp1_16x8 = vmull_u8(a1_8x8, d1_8x8);
         temp0_8x8 = vshrn_n_u16(temp0_16x8,8);
         temp1_8x8 = vshrn_n_u16(temp1_16x8,8);
         temp_8x16 = vcombine_u8(temp0_8x8, temp1_8x8);
         temp_32x4 = vreinterpretq_u32_u8(temp_8x16);

         // if alpha is 0, replace a*d with d
         cond_32x4 = vceqq_u32(a_32x4, x0_32x4);
         ad_32x4 = vbslq_u32(cond_32x4, d_32x4, temp_32x4);

         // add *s
         d_32x4 = vaddq_u32(s_32x4, ad_32x4);

         // save result
         vst1q_u32(d, d_32x4);
         m+=4;
         d+=4;
         s+=4;
         continue;
      }
      // load 4 elements from m
      m_32 = k;
      m_32x2 = vset_lane_u32(m_32, m_32x2, 0);

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

      // make m 32 bit wide
      m_8x8 = vreinterpret_u8_u32(m_32x2);
      m_16x8 = vmovl_u8(m_8x8);
      m_8x16 = vreinterpretq_u8_u16(m_16x8);
      m_8x8 = vget_low_u8(m_8x16);
      m_16x8 = vmovl_u8(m_8x8);
      m_32x4 = vreinterpretq_u32_u16(m_16x8);

      // place m into every 8 bit element of vector
      m_32x4 = vmulq_u32(m_32x4, x1_32x4);
      m_8x16 = vreinterpretq_u8_u32(m_32x4);
      m0_8x8 = vget_low_u8(m_8x16);
      m1_8x8 = vget_high_u8(m_8x16);

      // multiply MUL_SYM(m, *s);
      ms0_16x8 = vmull_u8(m0_8x8, s0_8x8);
      ms1_16x8 = vmull_u8(m1_8x8, s1_8x8);
      ms0_16x8 = vaddq_u16(ms0_16x8, x255_16x8);
      ms1_16x8 = vaddq_u16(ms1_16x8, x255_16x8);
      ms0_8x8 = vshrn_n_u16(ms0_16x8, 8);
      ms1_8x8 = vshrn_n_u16(ms1_16x8, 8);
      ms_8x16 = vcombine_u8(ms0_8x8, ms1_8x8);

      // substract 256 - m*s
      a_8x16 = vsubq_u8(x0_8x16, ms_8x16);
      a_32x4 = vreinterpretq_u32_u8(a_8x16);

      // shift alpha>>24 and place it into every 8bit element
      a_32x4 = vshrq_n_u32(a_32x4, 24);
      a_32x4 = vmulq_u32(a_32x4, x1_32x4);
      a_8x16 = vreinterpretq_u8_u32(a_32x4);
      a0_8x8 = vget_low_u8(a_8x16);
      a1_8x8 = vget_high_u8(a_8x16);

      // multiply MUL_256(a, *d)
      temp0_16x8 = vmull_u8(a0_8x8, d0_8x8);
      temp1_16x8 = vmull_u8(a1_8x8, d1_8x8);
      temp0_8x8 = vshrn_n_u16(temp0_16x8,8);
      temp1_8x8 = vshrn_n_u16(temp1_16x8,8);
      temp_8x16 = vcombine_u8(temp0_8x8, temp1_8x8);
      temp_32x4 = vreinterpretq_u32_u8(temp_8x16);

      // if alpha is 0, replace a*d with d
      cond_32x4 = vceqq_u32(a_32x4, x0_32x4);
      ad_32x4 = vbslq_u32(cond_32x4, d_32x4, temp_32x4);

      // add m*s
      ms_32x4 = vreinterpretq_u32_u8(ms_8x16);
      d_32x4 = vaddq_u32(ms_32x4, ad_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
      s+=4;
      m+=4;
   }

   int alpha;
   DATA32 temp;

   end += (l & 3);
   while (d < end)
   {
      alpha = *m;
      switch(alpha)
        {
        case 0:
           break;
        case 255:
           alpha = 256 - (*s >> 24);
           *d = *s + MUL_256(alpha, *d);
           break;
        default:
           temp = MUL_SYM(alpha, *s);
           alpha = 256 - (temp >> 24);
           *d = temp + MUL_256(alpha, *d);
           break;
        }
      m++;  s++;  d++;
   }
}

static void
_op_blend_p_mas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c EINA_UNUSED, DATA32 *d, int l) {
   uint16x8_t m_16x8;
   uint16x8_t ms0_16x8;
   uint16x8_t ms1_16x8;
   uint16x8_t temp0_16x8;
   uint16x8_t temp1_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t m_32x2;
   uint32x4_t a_32x4;
   uint32x4_t ad_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t d_32x4;
   uint32x4_t m_32x4;
   uint32x4_t ms_32x4;
   uint32x4_t s_32x4;
   uint32x4_t temp_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t a_8x16;
   uint8x16_t d_8x16;
   uint8x16_t m_8x16;
   uint8x16_t ms_8x16;
   uint8x16_t s_8x16;
   uint8x16_t temp_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t a0_8x8;
   uint8x8_t a1_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t m0_8x8;
   uint8x8_t m1_8x8;
   uint8x8_t m_8x8;
   uint8x8_t ms0_8x8;
   uint8x8_t ms1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;
   uint8x8_t temp0_8x8;
   uint8x8_t temp1_8x8;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x255_16x8 = vdupq_n_u16(0xff);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from m
      m_32x2 = vld1_lane_u32((DATA32*)m, m_32x2, 0);

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

      // make m 32 bit wide
      m_8x8 = vreinterpret_u8_u32(m_32x2);
      m_16x8 = vmovl_u8(m_8x8);
      m_8x16 = vreinterpretq_u8_u16(m_16x8);
      m_8x8 = vget_low_u8(m_8x16);
      m_16x8 = vmovl_u8(m_8x8);
      m_32x4 = vreinterpretq_u32_u16(m_16x8);

      // place m into every 8 bit element of vector
      m_32x4 = vmulq_u32(m_32x4, x1_32x4);
      m_8x16 = vreinterpretq_u8_u32(m_32x4);
      m0_8x8 = vget_low_u8(m_8x16);
      m1_8x8 = vget_high_u8(m_8x16);

      // multiply MUL_SYM(m, *s);
      ms0_16x8 = vmull_u8(m0_8x8, s0_8x8);
      ms1_16x8 = vmull_u8(m1_8x8, s1_8x8);
      ms0_16x8 = vaddq_u16(ms0_16x8, x255_16x8);
      ms1_16x8 = vaddq_u16(ms1_16x8, x255_16x8);
      ms0_8x8 = vshrn_n_u16(ms0_16x8, 8);
      ms1_8x8 = vshrn_n_u16(ms1_16x8, 8);
      ms_8x16 = vcombine_u8(ms0_8x8, ms1_8x8);

      // substract 256 - m*s
      a_8x16 = vsubq_u8(x0_8x16, ms_8x16);
      a_32x4 = vreinterpretq_u32_u8(a_8x16);

      // shift alpha>>24 and place it into every 8bit element
      a_32x4 = vshrq_n_u32(a_32x4, 24);
      a_32x4 = vmulq_u32(a_32x4, x1_32x4);
      a_8x16 = vreinterpretq_u8_u32(a_32x4);
      a0_8x8 = vget_low_u8(a_8x16);
      a1_8x8 = vget_high_u8(a_8x16);

      // multiply MUL_256(a, *d)
      temp0_16x8 = vmull_u8(a0_8x8, d0_8x8);
      temp1_16x8 = vmull_u8(a1_8x8, d1_8x8);
      temp0_8x8 = vshrn_n_u16(temp0_16x8,8);
      temp1_8x8 = vshrn_n_u16(temp1_16x8,8);
      temp_8x16 = vcombine_u8(temp0_8x8, temp1_8x8);
      temp_32x4 = vreinterpretq_u32_u8(temp_8x16);

      // if alpha is 0, replace a*d with d
      cond_32x4 = vceqq_u32(a_32x4, x0_32x4);
      ad_32x4 = vbslq_u32(cond_32x4, d_32x4, temp_32x4);

      // add m*s
      ms_32x4 = vreinterpretq_u32_u8(ms_8x16);
      d_32x4 = vaddq_u32(ms_32x4, ad_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
      s+=4;
      m+=4;
   }

   int alpha;
   DATA32 temp;

   end += (l & 3);
   while (d < end)
   {
      alpha = *m;
      temp = MUL_SYM(alpha, *s);
      alpha = 256 - (temp >> 24);
      *d = temp + MUL_256(alpha, *d);
      m++;  s++;  d++;
   }
}

#define _op_blend_pan_mas_dp_neon _op_blend_pas_mas_dp_neon

#define _op_blend_p_mas_dpan_neon _op_blend_p_mas_dp_neon
#define _op_blend_pan_mas_dpan_neon _op_blend_pan_mas_dp_neon
#define _op_blend_pas_mas_dpan_neon _op_blend_pas_mas_dp_neon

static void
init_blend_pixel_mask_span_funcs_neon(void)
{
   op_blend_span_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_p_mas_dp_neon;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pas_mas_dp_neon;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pan_mas_dp_neon;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_p_mas_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pas_mas_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pan_mas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_p_mas_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   s = MUL_SYM(m, s);
   c = 256 - (s >> 24);
   *d = s + MUL_256(c, *d);
}

#define _op_blend_pt_pan_mas_dp_neon _op_blend_pt_p_mas_dp_neon
#define _op_blend_pt_pas_mas_dp_neon _op_blend_pt_p_mas_dp_neon

#define _op_blend_pt_p_mas_dpan_neon _op_blend_pt_p_mas_dp_neon
#define _op_blend_pt_pas_mas_dpan_neon _op_blend_pt_pas_mas_dp_neon
#define _op_blend_pt_pan_mas_dpan_neon _op_blend_pt_pan_mas_dp_neon

static void
init_blend_pixel_mask_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pt_p_mas_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pt_pas_mas_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pt_pan_mas_dp_neon;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_p_mas_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_pas_mas_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_pan_mas_dpan_neon;
}
#endif

/*-----*/

/* blend_rel pixel x mask -> dst */

#ifdef BUILD_NEON

#define _op_blend_rel_p_mas_dpan_neon _op_blend_p_mas_dpan_neon
#define _op_blend_rel_pas_mas_dpan_neon _op_blend_pas_mas_dpan_neon
#define _op_blend_rel_pan_mas_dpan_neon _op_blend_pan_mas_dpan_neon

static void
init_blend_rel_pixel_mask_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_p_mas_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pas_mas_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pan_mas_dpan_neon;
}
#endif

#ifdef BUILD_NEON

#define _op_blend_rel_pt_p_mas_dpan_neon _op_blend_pt_p_mas_dpan_neon
#define _op_blend_rel_pt_pas_mas_dpan_neon _op_blend_pt_pas_mas_dpan_neon
#define _op_blend_rel_pt_pan_mas_dpan_neon _op_blend_pt_pan_mas_dpan_neon

static void
init_blend_rel_pixel_mask_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_mas_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_mas_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_mas_dpan_neon;
}
#endif
