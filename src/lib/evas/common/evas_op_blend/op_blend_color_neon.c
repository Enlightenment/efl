#ifdef BUILD_NEON
#include <arm_neon.h>
#endif
/* blend color --> dst */

#ifdef BUILD_NEON
static void
_op_blend_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
#ifdef BUILD_NEON_INTRINSICS
   uint16x8_t temp00_16x8;
   uint16x8_t temp01_16x8;
   uint16x8_t temp10_16x8;
   uint16x8_t temp11_16x8;
   uint32x4_t temp0_32x4;
   uint32x4_t temp1_32x4;
   uint32x4_t c_32x4;
   uint32x4_t d0_32x4;
   uint32x4_t d1_32x4;
   uint8x16_t d0_8x16;
   uint8x16_t d1_8x16;
   uint8x16_t temp0_8x16;
   uint8x16_t temp1_8x16;
   uint8x8_t alpha_8x8;
   uint8x8_t d00_8x8;
   uint8x8_t d01_8x8;
   uint8x8_t d10_8x8;
   uint8x8_t d11_8x8;
   uint8x8_t temp00_8x8;
   uint8x8_t temp01_8x8;
   uint8x8_t temp10_8x8;
   uint8x8_t temp11_8x8;

   // alpha can only be 0 if color is 0x0. In that case we can just return.
   // Otherwise we can assume alpha != 0. This allows more optimization in
   // NEON code.

   if(!c)
      return;

   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~7);

   unsigned char alpha;
   alpha = ~(c >> 24) + 1; // 256 - (c >> 24)
   alpha_8x8 = vdup_n_u8(alpha);

   c_32x4 = vdupq_n_u32(c);

   while (start < end)
   {
      d0_32x4 = vld1q_u32(start);
      d1_32x4 = vld1q_u32(start+4);
      d0_8x16 = vreinterpretq_u8_u32(d0_32x4);
      d1_8x16 = vreinterpretq_u8_u32(d1_32x4);

      d00_8x8 = vget_low_u8(d0_8x16);
      d01_8x8 = vget_high_u8(d0_8x16);
      d10_8x8 = vget_low_u8(d1_8x16);
      d11_8x8 = vget_high_u8(d1_8x16);

      temp00_16x8 = vmull_u8(alpha_8x8, d00_8x8);
      temp01_16x8 = vmull_u8(alpha_8x8, d01_8x8);
      temp10_16x8 = vmull_u8(alpha_8x8, d10_8x8);
      temp11_16x8 = vmull_u8(alpha_8x8, d11_8x8);

      temp00_8x8 = vshrn_n_u16(temp00_16x8,8);
      temp01_8x8 = vshrn_n_u16(temp01_16x8,8);
      temp10_8x8 = vshrn_n_u16(temp10_16x8,8);
      temp11_8x8 = vshrn_n_u16(temp11_16x8,8);

      temp0_8x16 = vcombine_u8(temp00_8x8, temp01_8x8);
      temp1_8x16 = vcombine_u8(temp10_8x8, temp11_8x8);

      temp0_32x4 = vreinterpretq_u32_u8(temp0_8x16);
      temp1_32x4 = vreinterpretq_u32_u8(temp1_8x16);

      d0_32x4 = vaddq_u32(c_32x4, temp0_32x4);
      d1_32x4 = vaddq_u32(c_32x4, temp1_32x4);

      vst1q_u32(start, d0_32x4);
      vst1q_u32(start+4, d1_32x4);
      start+=8;
   }
   end += (size & 7);
   while (start <  end)
   {
      *start = c + MUL_256(alpha, *start);
      start++;
   }
#else
	DATA32 *e, *tmp = 0;
#define AP	"B_C_DP"
   asm volatile (
	".fpu neon					\n\t"
	"vdup.u32	q6, %[c]			\n\t"
	"vmov.i8	q5, #1				\n\t"
	"vmvn.u8	q7,q6				\n\t"
	"vshr.u32	q7, q7, $0x18			\n\t"
	"vmul.u32	q7,q5, q7			\n\t"
	"bic		%[e], #3			\n\t"
	"bic		%[d], #3			\n\t"

	AP "loopchoose:					\n\t"
		// If aligned already - straight to quads
		"andS		%[tmp], %[d],$0x1f		\n\t"
		"beq		"AP"quadloops			\n\t"

		"andS		%[tmp], %[d],$0x4		\n\t"
		"beq		"AP"dualloop			\n\t"

	// Only ever executes once, fall through to dual
	AP "singleloop:					\n\t"
		// Use 'tmp' not 'd'
		"vld1.32	d0[0], [%[d]]		\n\t"
		// Only touch d1
		"vmovl.u8	q10, d0			\n\t"
		"vmull.u8	q0, d0, d14		\n\t"
		"vadd.u16	q0, q0, q10		\n\t"
		"vshrn.u16	d0, q0, #8		\n\t"
		"vadd.u8	d0, d12, d0		\n\t"
		"vst1.32	d0[0], [%[d]]		\n\t"

		"add		%[d], #4		\n\t"

		// Can we go the fast path?
		"andS		%[tmp], %[d],$0x1f	\n\t"
		"beq		"AP"quadloops		\n\t"

	AP "dualloop:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"blt		"AP"loopout			\n\t"


	AP "dualloopint:					\n\t"
		"vldr.32	d0, [%[d]]		\n\t"
		"vmovl.u8	q10, d0			\n\t"
		"vmull.u8	q1, d0, d14		\n\t"
		"vadd.u16	q1, q1, q10		\n\t"
		"vshrn.u16	d0, q1, #8		\n\t"
		"vqadd.u8	d0, d0, d12		\n\t"

		"vstm		%[d]!, {d0}		\n\t"

		"ands		%[tmp], %[d], $0x1f	\n\t"
		"bne 		"AP"dualloopint		\n\t"

	AP "quadloops:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"blt		"AP"loopout			\n\t"

		"sub		%[tmp],%[e],#31	\n\t"

	AP "quadloopint:\n\t"
		"vldm	%[d],	{d0,d1,d2,d3}		\n\t"

		"vmovl.u8	q10, d0			\n\t"
		"vmovl.u8	q11, d1			\n\t"
		"vmovl.u8	q12, d2			\n\t"
		"vmovl.u8	q13, d3			\n\t"
		"vmull.u8	q2, d0, d14		\n\t"
		"vmull.u8	q3, d1, d15		\n\t"
		"vmull.u8	q4, d2, d14		\n\t"
		"vmull.u8	q5, d3, d15		\n\t"
		"vadd.u16	q2, q2, q10		\n\t"
		"vadd.u16	q3, q3, q11		\n\t"
		"vadd.u16	q4, q4, q12		\n\t"
		"vadd.u16	q5, q5, q13		\n\t"

		"vshrn.u16	d0, q2, #8		\n\t"
		"vshrn.u16	d1, q3, #8		\n\t"
		"vshrn.u16	d2, q4, #8		\n\t"
		"vshrn.u16	d3, q5, #8		\n\t"

		"vqadd.u8	q0, q6, q0		\n\t"
		"vqadd.u8	q1, q6, q1		\n\t"

		"vstm	%[d]!,	{d0,d1,d2,d3}		\n\t"

		"cmp	 %[tmp], %[d]\n\t"
                "bhi "AP"quadloopint\n\t"

	AP "loopout:					\n\t"
		"cmp 		%[d], %[e]\n\t"
                "beq 		"AP"done\n\t"
		"sub		%[tmp],%[e], %[d]	\n\t"
		"cmp		%[tmp],#8		\n\t"
		"blt		"AP"singleloop2		\n\t"

	AP "dualloop2:					\n\t"
		"sub		%[tmp],%[e],$0x7	\n\t"
	AP "dualloop2int:					\n\t"
		"vldr.64	d0, [%[d]]		\n\t"
		"vmovl.u8	q10, d0			\n\t"
		"vmull.u8	q1, d0, d14		\n\t"
		"vadd.u16	q1, q1, q10		\n\t"
		"vshrn.u16	d0, q1, #8		\n\t"
		"vqadd.u8	d0, d0, d12		\n\t"

		"vstr.64	d0, [%[d]]		\n\t"

		"add		%[d], #8		\n\t"
		"cmp 		%[tmp], %[d]		\n\t"
		"bhi 		"AP"dualloop2int		\n\t"

		// Single ??
		"cmp 		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP "singleloop2:					\n\t"
		"vld1.32	d0[0], [%[d]]		\n\t"
		"vmovl.u8	q10, d0			\n\t"
		"vmull.u8	q1, d0, d14		\n\t"
		"vadd.u16	q1, q1, q10		\n\t"
		"vshrn.u16	d0, q1, #8		\n\t"
		"vqadd.u8	d0, d0, d12		\n\t"

		"vst1.32	d0[0], [%[d]]		\n\t"

	AP "done:\n\t"

	  : // output regs
	  // Input
          :  [e] "r" (e = d + l), [d] "r" (d), [c] "r" (c), [tmp] "r" (tmp)
          : "q0", "q1", "q2","q3", "q4","q5","q6", "q7", "q10", "q11", "q12", "q13", "memory" // clobbered

	);
#undef AP
#endif
}

#define _op_blend_caa_dp_neon _op_blend_c_dp_neon

#define _op_blend_c_dpan_neon _op_blend_c_dp_neon
#define _op_blend_caa_dpan_neon _op_blend_c_dpan_neon

static void
init_blend_color_span_funcs_neon(void)
{
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_c_dp_neon;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_caa_dp_neon;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_c_dpan_neon;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_c_dp_neon(DATA32 s EINA_UNUSED, DATA8 m EINA_UNUSED, DATA32 c, DATA32 *d) {
   s = 256 - (c >> 24);
   *d = c + MUL_256(s, *d);
}

#define _op_blend_pt_caa_dp_neon _op_blend_pt_c_dp_neon

#define _op_blend_pt_c_dpan_neon _op_blend_pt_c_dp_neon
#define _op_blend_pt_caa_dpan_neon _op_blend_pt_c_dpan_neon

static void
init_blend_color_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_c_dp_neon;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_caa_dp_neon;

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_c_dpan_neon;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_caa_dpan_neon;
}
#endif
/*-----*/

/* blend_rel color -> dst */

#ifdef BUILD_NEON
static void
_op_blend_rel_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t ad0_16x8;
   uint16x8_t ad1_16x8;
   uint16x8_t dc0_16x8;
   uint16x8_t dc1_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t c_32x2;
   uint32x4_t ad_32x4;
   uint32x4_t d_32x4;
   uint32x4_t dc_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad_8x16;
   uint8x16_t d_8x16;
   uint8x16_t dc_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad0_8x8;
   uint8x8_t ad1_8x8;
   uint8x8_t alpha_8x8;
   uint8x8_t c_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t dc0_8x8;
   uint8x8_t dc1_8x8;

   // alpha can only be 0 if color is 0x0. In that case we can just return.
   // Otherwise we can assume alpha != 0. This allows more optimization in
   // NEON code.

   if(!c)
      return;

   unsigned char alpha;
   alpha = ~(c >> 24) + 1; // 256 - (c >> 24)

   alpha_8x8 = vdup_n_u8(alpha);
   c_32x2 = vdup_n_u32(c);
   c_8x8 = vreinterpret_u8_u32(c_32x2);
   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x255_16x8 = vdupq_n_u16(0xff);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from d
      d_32x4 = vld1q_u32(d);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      // multiply MUL_256(alpha, *d);
      ad0_16x8 = vmull_u8(alpha_8x8, d0_8x8);
      ad1_16x8 = vmull_u8(alpha_8x8, d1_8x8);
      ad0_8x8 = vshrn_n_u16(ad0_16x8,8);
      ad1_8x8 = vshrn_n_u16(ad1_16x8,8);
      ad_8x16 = vcombine_u8(ad0_8x8, ad1_8x8);
      ad_32x4 = vreinterpretq_u32_u8(ad_8x16);

      // shift (*d >> 24)
      dc_32x4 = vshrq_n_u32(d_32x4, 24);
      dc_32x4 = vmulq_u32(x1_32x4, dc_32x4);
      dc_8x16 = vreinterpretq_u8_u32(dc_32x4);
      dc0_8x8 = vget_low_u8(dc_8x16);
      dc1_8x8 = vget_high_u8(dc_8x16);

      // multiply MUL_256(*d >> 24, sc);
      dc0_16x8 = vmull_u8(dc0_8x8, c_8x8);
      dc1_16x8 = vmull_u8(dc1_8x8, c_8x8);
      dc0_16x8 = vaddq_u16(dc0_16x8, x255_16x8);
      dc1_16x8 = vaddq_u16(dc1_16x8, x255_16x8);
      dc0_8x8 = vshrn_n_u16(dc0_16x8, 8);
      dc1_8x8 = vshrn_n_u16(dc1_16x8, 8);
      dc_8x16 = vcombine_u8(dc0_8x8, dc1_8x8);

      // add up everything
      dc_32x4 = vreinterpretq_u32_u8(dc_8x16);
      d_32x4 = vaddq_u32(dc_32x4, ad_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
   }

   end += (l & 3);
   while (d < end)
   {
      *d = MUL_SYM(*d >> 24, c) + MUL_256(alpha, *d);
      d++;
   }
}

#define _op_blend_rel_caa_dp_neon _op_blend_rel_c_dp_neon

#define _op_blend_rel_c_dpan_neon _op_blend_c_dpan_neon
#define _op_blend_rel_caa_dpan_neon _op_blend_caa_dpan_neon

static void
init_blend_rel_color_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_rel_c_dp_neon;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_rel_caa_dp_neon;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_c_dpan_neon;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_rel_pt_c_dp_neon(DATA32 s EINA_UNUSED, DATA8 m EINA_UNUSED, DATA32 c, DATA32 *d) {
   s = *d >> 24;
   *d = MUL_SYM(s, c) + MUL_256(256 - (c >> 24), *d);
}

#define _op_blend_rel_pt_caa_dp_neon _op_blend_rel_pt_c_dp_neon

#define _op_blend_rel_pt_c_dpan_neon _op_blend_pt_c_dpan_neon
#define _op_blend_rel_pt_caa_dpan_neon _op_blend_pt_caa_dpan_neon

static void
init_blend_rel_color_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_rel_pt_c_dp_neon;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_rel_pt_caa_dp_neon;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_c_dpan_neon;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_caa_dpan_neon;
}
#endif
