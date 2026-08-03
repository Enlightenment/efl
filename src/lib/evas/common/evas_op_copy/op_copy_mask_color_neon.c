#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

/* copy mask x color -> dst */

#ifdef BUILD_NEON
#ifdef BUILD_NEON_INTRINSICS
/* The AArch64 path below uses tbl and the across-vector reduce, neither of
 * which exists in 32 bit NEON. Keep the original kernel for ARMv7 rather
 * than adding a second version that cannot be tested here. */
static void
_op_copy_mas_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   /* d = MUL_256(m + 1, c), which per channel is (c * (m + 1)) >> 8.
    *
    * All three of the C reference's cases fall out of that one expression, so
    * no branching or lane selection is needed: m = 0 gives (c * 1) >> 8, and
    * every channel of c is below 256, so that is 0; m = 255 gives
    * (c * 256) >> 8, which is c.
    *
    * There is no destination read and no inter-channel carry to preserve
    * either - MUL_256 masks each channel and its two halves live in disjoint
    * bits - so this is just a multiply and a narrowing shift per pixel. The
    * only awkward part is replicating each mask byte across its pixel's four
    * channels, and that is one tbl against a constant index vector. */
   static const uint8_t splat_idx[4][16] = {
      {  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3 },
      {  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7 },
      {  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11 },
      { 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15 }
   };
   const uint8x16_t c8 = vreinterpretq_u8_u32(vdupq_n_u32(c));
   const uint16x8_t clo = vmovl_u8(vget_low_u8(c8));
   const uint16x8_t chi = vmovl_u8(vget_high_u8(c8));
   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~15);

   while (start < end)
     {
        uint8x16_t m16 = vld1q_u8(m);
        int j;

        for (j = 0; j < 4; j++)
          {
             uint8x16_t m8 = vqtbl1q_u8(m16, vld1q_u8(splat_idx[j]));
             /* c * m + c == c * (m + 1) */
             uint16x8_t lo = vmlal_u8(clo, vget_low_u8(c8), vget_low_u8(m8));
             uint16x8_t hi = vmlal_u8(chi, vget_high_u8(c8), vget_high_u8(m8));

             vst1q_u8((uint8_t *)(start + (j * 4)),
                      vcombine_u8(vshrn_n_u16(lo, 8), vshrn_n_u16(hi, 8)));
          }

        m += 16;
        start += 16;
     }

   end += (size & 15);
   while (start < end) {
      DATA32 alpha = *m;

      switch (alpha)
        {
         case 0:
            *start = 0;
            break;
         case 255:
            *start = c;
            break;
         default:
            alpha++;
            *start = MUL_256(alpha, c);
            break;
        }
      m++;  start++;
   }
}
#else
static void
_op_copy_mas_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        /* d = m*c */
                        alpha = *m;
                        switch(alpha)
                          {
                          case 0:
                             *d = 0;
                             break;
                          case 255:
                             *d = c;
                             break;
                          default:
                             alpha++;
                             *d = MUL_256(alpha, c);
                             break;
                          }
                        m++;  d++;
                     });
}
#endif

#define _op_copy_mas_cn_dp_neon _op_copy_mas_c_dp_neon
#define _op_copy_mas_can_dp_neon _op_copy_mas_c_dp_neon
#define _op_copy_mas_caa_dp_neon _op_copy_mas_c_dp_neon

#define _op_copy_mas_c_dpan_neon _op_copy_mas_c_dp_neon
#define _op_copy_mas_cn_dpan_neon _op_copy_mas_c_dpan_neon
#define _op_copy_mas_can_dpan_neon _op_copy_mas_c_dpan_neon
#define _op_copy_mas_caa_dpan_neon _op_copy_mas_c_dpan_neon

static void
init_copy_mask_color_span_funcs_neon(void)
{
   op_copy_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_mas_cn_dp_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_copy_mas_c_dp_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_copy_mas_can_dp_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_copy_mas_caa_dp_neon;

   op_copy_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_mas_cn_dpan_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_mas_c_dpan_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_mas_can_dpan_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_mas_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_mas_c_dp_neon(DATA32 s EINA_UNUSED, DATA8 m, DATA32 c, DATA32 *d) {
   *d = INTERP_256(m + 1, c, *d);
}

#define _op_copy_pt_mas_cn_dp_neon _op_copy_pt_mas_c_dp_neon
#define _op_copy_pt_mas_can_dp_neon _op_copy_pt_mas_c_dp_neon
#define _op_copy_pt_mas_caa_dp_neon _op_copy_pt_mas_c_dp_neon

#define _op_copy_pt_mas_c_dpan_neon _op_copy_pt_mas_c_dp_neon
#define _op_copy_pt_mas_cn_dpan_neon _op_copy_pt_mas_c_dpan_neon
#define _op_copy_pt_mas_can_dpan_neon _op_copy_pt_mas_c_dpan_neon
#define _op_copy_pt_mas_caa_dpan_neon _op_copy_pt_mas_c_dpan_neon

static void
init_copy_mask_color_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_pt_mas_cn_dp_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_copy_pt_mas_c_dp_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_copy_pt_mas_can_dp_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_copy_pt_mas_caa_dp_neon;

   op_copy_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_mas_cn_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_pt_mas_c_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_pt_mas_can_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_pt_mas_caa_dpan_neon;
}
#endif

/*-----*/

/* copy_rel mask x color -> dst */

#ifdef BUILD_NEON


#define _op_copy_rel_mas_c_dpan_neon _op_copy_mas_c_dpan_neon
#define _op_copy_rel_mas_cn_dpan_neon _op_copy_mas_cn_dpan_neon
#define _op_copy_rel_mas_can_dpan_neon _op_copy_mas_can_dpan_neon
#define _op_copy_rel_mas_caa_dpan_neon _op_copy_mas_caa_dpan_neon

static void
init_copy_rel_mask_color_span_funcs_neon(void)
{

   op_copy_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_mas_cn_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_rel_mas_c_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_mas_can_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_mas_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON


#define _op_copy_rel_pt_mas_c_dpan_neon _op_copy_pt_mas_c_dpan_neon
#define _op_copy_rel_pt_mas_cn_dpan_neon _op_copy_pt_mas_cn_dpan_neon
#define _op_copy_rel_pt_mas_can_dpan_neon _op_copy_pt_mas_can_dpan_neon
#define _op_copy_rel_pt_mas_caa_dpan_neon _op_copy_pt_mas_caa_dpan_neon

static void
init_copy_rel_mask_color_pt_funcs_neon(void)
{

   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_cn_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_c_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_can_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_caa_dpan_neon;
}
#endif
