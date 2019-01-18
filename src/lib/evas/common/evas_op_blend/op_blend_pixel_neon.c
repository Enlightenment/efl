#ifdef BUILD_NEON
#include <arm_neon.h>
#endif
/* blend pixel --> dst */

#ifdef BUILD_NEON
static void
_op_blend_p_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d, int l) {
#ifdef BUILD_NEON_INTRINSICS
   uint16x8_t ad00_16x8;
   uint16x8_t ad01_16x8;
   uint16x8_t ad10_16x8;
   uint16x8_t ad11_16x8;
   uint32x4_t ad0_32x4;
   uint32x4_t ad1_32x4;
   uint32x4_t alpha0_32x4;
   uint32x4_t alpha1_32x4;
   uint32x4_t cond0_32x4;
   uint32x4_t cond1_32x4;
   uint32x4_t d0_32x4;
   uint32x4_t d1_32x4;
   uint32x4_t s0_32x4;
   uint32x4_t s1_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad0_8x16;
   uint8x16_t ad1_8x16;
   uint8x16_t alpha0_8x16;
   uint8x16_t alpha1_8x16;
   uint8x16_t d0_8x16;
   uint8x16_t d1_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad00_8x8;
   uint8x8_t ad01_8x8;
   uint8x8_t ad10_8x8;
   uint8x8_t ad11_8x8;
   uint8x8_t alpha00_8x8;
   uint8x8_t alpha01_8x8;
   uint8x8_t alpha10_8x8;
   uint8x8_t alpha11_8x8;
   uint8x8_t d00_8x8;
   uint8x8_t d01_8x8;
   uint8x8_t d10_8x8;
   uint8x8_t d11_8x8;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);

   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~7);
   while (start < end)
   {
      s0_32x4 = vld1q_u32(s);
      s1_32x4 = vld1q_u32(s+4);

      d0_32x4 = vld1q_u32(start);
      d1_32x4 = vld1q_u32(start+4);

      d0_8x16 = vreinterpretq_u8_u32(d0_32x4);
      d1_8x16 = vreinterpretq_u8_u32(d1_32x4);
      d00_8x8 = vget_low_u8(d0_8x16);
      d01_8x8 = vget_high_u8(d0_8x16);
      d10_8x8 = vget_low_u8(d1_8x16);
      d11_8x8 = vget_high_u8(d1_8x16);

      alpha0_32x4 = vshrq_n_u32(s0_32x4, 24);
      alpha1_32x4 = vshrq_n_u32(s1_32x4, 24);

      alpha0_32x4 = vmulq_u32(x1_32x4, alpha0_32x4);
      alpha1_32x4 = vmulq_u32(x1_32x4, alpha1_32x4);

      alpha0_8x16 = vreinterpretq_u8_u32(alpha0_32x4);
      alpha1_8x16 = vreinterpretq_u8_u32(alpha1_32x4);

      alpha0_8x16 = vsubq_u8(x0_8x16, alpha0_8x16);
      alpha1_8x16 = vsubq_u8(x0_8x16, alpha1_8x16);

      alpha0_32x4 = vreinterpretq_u32_u8(alpha0_8x16);
      alpha1_32x4 = vreinterpretq_u32_u8(alpha1_8x16);

      alpha10_8x8 = vget_low_u8(alpha1_8x16);
      alpha11_8x8 = vget_high_u8(alpha1_8x16);
      alpha00_8x8 = vget_low_u8(alpha0_8x16);
      alpha01_8x8 = vget_high_u8(alpha0_8x16);

      ad00_16x8 = vmull_u8(alpha00_8x8, d00_8x8);
      ad01_16x8 = vmull_u8(alpha01_8x8, d01_8x8);
      ad10_16x8 = vmull_u8(alpha10_8x8, d10_8x8);
      ad11_16x8 = vmull_u8(alpha11_8x8, d11_8x8);
      ad00_8x8 = vshrn_n_u16(ad00_16x8,8);
      ad01_8x8 = vshrn_n_u16(ad01_16x8,8);
      ad10_8x8 = vshrn_n_u16(ad10_16x8,8);
      ad11_8x8 = vshrn_n_u16(ad11_16x8,8);

      ad0_8x16 = vcombine_u8(ad00_8x8, ad01_8x8);
      ad1_8x16 = vcombine_u8(ad10_8x8, ad11_8x8);
      ad0_32x4 = vreinterpretq_u32_u8(ad0_8x16);
      ad1_32x4 = vreinterpretq_u32_u8(ad1_8x16);

      cond0_32x4 = vceqq_u32(alpha0_32x4, x0_32x4);
      cond1_32x4 = vceqq_u32(alpha1_32x4, x0_32x4);

      ad0_32x4 = vbslq_u32(cond0_32x4, d0_32x4, ad0_32x4);
      ad1_32x4 = vbslq_u32(cond1_32x4, d1_32x4, ad1_32x4);

      d0_32x4 = vaddq_u32(s0_32x4, ad0_32x4);
      d1_32x4 = vaddq_u32(s1_32x4, ad1_32x4);

      vst1q_u32(start, d0_32x4);
      vst1q_u32(start+4, d1_32x4);

      s+=8;
      start+=8;
   }
   end += (size & 7);
   while (start <  end)
   {
      int alpha;
      alpha = 256 - (*s >> 24);
      *start = *s++ + MUL_256(alpha, *start);
      start++;
   }
#else
#define AP "blend_p_dp_"
  asm volatile (
	".fpu neon					\n\t"
	//** init
		"vmov.i8	q8,	$0x1		\n\t"

	AP "loopchoose:					\n\t"
		// If aligned already - straight to octs
		"andS		%[tmp], %[d],$0x1f	\n\t"
		"beq		"AP"octloops		\n\t"

		"andS		%[tmp], %[d],$0xf	\n\t"
		"beq		"AP"quadloops		\n\t"

		"andS		%[tmp], %[d],$0x4	\n\t"
		"beq		"AP"dualloop		\n\t"

	// Only ever executes once, fall through to dual
	AP "singleloop:					\n\t"
		"vld1.32	d0[0],	[%[s]]!		\n\t"
		"vld1.32	d4[0],  [%[d]]		\n\t"

		"vmvn.u8	d8, 	d0		\n\t"
		"vshr.u32	d8, 	d8, #24		\n\t"

		"vmul.u32	d8,	d16, d8		\n\t"

		"vmovl.u8	q9,	d4		\n\t"
		"vmull.u8	q6,	d4,d8		\n\t"
		"vadd.u16	q6,	q6, q9		\n\t"
		"vshrn.u16	d8,	q6, #8		\n\t"
		// Add to 's'
		"vqadd.u8	q2,	q4,q0		\n\t"

		"vst1.32	d4[0],	[%[d]]		\n\t"
		"add		%[d],	#4		\n\t"

		// Can we go the fast path?
		"andS		%[tmp], %[d],$0x1f	\n\t"
		"beq		"AP"octloops		\n\t"

		"andS		%[tmp], %[d],$0x0f	\n\t"
		"beq		"AP"quadloops		\n\t"


	AP "dualloop:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"blt		"AP"loopout		\n\t"

	AP "dualloopint:				\n\t"
		//** Dual Loop
		"vldm		%[s]!, {d0}		\n\t"
		"vldr		d4,	[%[d]]		\n\t"

		"vmvn.u8	d8, 	d0		\n\t"
		"vshr.u32	d8, 	d8, #24		\n\t"

		"vmul.u32	d8,	d16, d8		\n\t"

		"vmovl.u8	q9,	d4		\n\t"
		"vmull.u8	q6,	d4,d8		\n\t"
		"vadd.u16	q6,	q6, q9		\n\t"
		"vshrn.u16	d8,	q6, #8		\n\t"
		// Add to 's'
		"vqadd.u8	d4,	d8,d0		\n\t"
		"vstr		d4,	[%[d]]		\n\t"
		"add		%[d], 	#8		\n\t"

		"ands		%[tmp], %[d], $0x1f	\n\t"
		"beq 		"AP"octloops		\n\t"

	AP"quadloops:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"blt		"AP"loopout		\n\t"

		"vldm		%[s]!,	{d0,d1)		\n\t"
		"vldm		%[d],	{d4,d5}		\n\t"


		// Copy s.a into q2 (>> 24) & subtract from 255
		"vmvn.u8	q4,	q0		\n\t"
		"vshr.u32	q4,	q4,$0x18	\n\t"

		// Multiply into all fields
		"vmul.u32	q4,	q8,q4		\n\t"

		"vmovl.u8	q9,	d4		\n\t"
		"vmovl.u8	q10,	d5		\n\t"
		// a * d  (clobbering 'd'/q7)
		"vmull.u8	q6,	d4,d8		\n\t"
		"vmull.u8	q2,	d5,d9		\n\t"

		"vadd.u16	q6,	q6, q9		\n\t"
		"vadd.u16	q2,	q2, q10		\n\t"

		// Shift & narrow it
		"vshrn.u16	d8,	q6, #8		\n\t"
		"vshrn.u16	d9,	q2, #8		\n\t"

		// Add to s
		"vqadd.u8	q2,	q4,q0		\n\t"

		// Write it
		"vstm		%[d]!,	{d4,d5}		\n\t"

	AP "octloops:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"ble		"AP"loopout		\n\t"

		"sub		%[tmp],%[e],#64		\n\t"


	AP "octloopint:\n\t"
		//** Oct loop
		"vldm	%[s]!,	{d0,d1,d2,d3)		\n\t"
		"vldm	%[d],	{d4,d5,d6,d7}		\n\t"
                "pld    [%[s], #64]                     \n\t"
                

		// Copy s.a into q2 (>> 24) & subtract from 255
		"vmvn.u8	q4,	q0		\n\t"
			"vmvn.u8	q5,	q1	\n\t"
		"vshr.u32	q4,	q4,$0x18	\n\t"
			"vshr.u32	q5,	q5,$0x18\n\t"

		// Multiply into all fields
		"vmul.u32	q4,	q8,q4		\n\t"
			"vmul.u32	q5,	q8,q5	\n\t"

		"vmovl.u8	q9,	d4		\n\t"
		"vmovl.u8	q10,	d5		\n\t"
		"vmovl.u8	q11,	d6		\n\t"
		"vmovl.u8	q12,	d7		\n\t"

		// a * d  (clobbering 'd'/q7)
		"vmull.u8	q6,	d4,d8		\n\t"
		"vmull.u8	q2,	d5,d9		\n\t"
			"vmull.u8	q7,	d6,d10	\n\t"
			"vmull.u8	q3,	d7,d11	\n\t"

		"vadd.u16	q6,	q6, q9		\n\t"
		"vadd.u16	q2,	q2, q10		\n\t"
		"vadd.u16	q7,	q7, q11		\n\t"
		"vadd.u16	q3,	q3, q12		\n\t"

		"cmp	 %[tmp], %[d]\n\t"

		// Shift & narrow it
		"vshrn.u16	d8,	q6, #8		\n\t"
		"vshrn.u16	d9,	q2, #8		\n\t"
			"vshrn.u16	d10,	q7, #8	\n\t"
			"vshrn.u16	d11,	q3, #8	\n\t"


		// Add to s
		"vqadd.u8	q2,	q4,q0		\n\t"
			"vqadd.u8	q3,	q5,q1	\n\t"

		// Write it
		"vstm		%[d]!,	{d4,d5,d6,d7}	\n\t"

                "bhi	 "AP"octloopint\n\t"

	AP "loopout:					\n\t"
		"cmp 		%[d], %[e]		\n\t"
                "beq 		"AP"done		\n\t"
		"sub		%[tmp],%[e], %[d]	\n\t"
		"cmp		%[tmp],$0x04		\n\t"
		"ble		"AP"singleloop2		\n\t"

	AP "dualloop2:					\n\t"
		"sub		%[tmp],%[e],$0x7	\n\t"
	AP "dualloop2int:				\n\t"
		//** Trailing double
	
		"vldm		%[s]!,	{d0}		\n\t"
		"vldm		%[d],	{d4}		\n\t"

		"vmvn.u8	d8, 	d0		\n\t"
		"vshr.u32	d8, 	d8, #24		\n\t"

		"vmul.u32	d8,	d16, d8		\n\t"

		"vmovl.u8	q9,	d4		\n\t"
		"vmull.u8	q6,	d4,d8		\n\t"
		"vadd.u16	q6,	q6, q9		\n\t"
		"vshrn.u16	d8,	q6, #8		\n\t"
		// Add to 's'
		"vqadd.u8	d4,	d8,d0		\n\t"

		"vstr.32	d4,	[%[d]]		\n\t"
		"add		%[d],   #8		\n\t"

		"cmp 		%[tmp], %[d]		\n\t"
		"bhi 		"AP"dualloop2int	\n\t"

		// Single ??
		"cmp 		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP"singleloop2:					\n\t"
		"vld1.32	d0[0],	[%[s]]		\n\t"
		"vld1.32	d4[0],  [%[d]]		\n\t"

		"vmvn.u8	d8, 	d0		\n\t"
		"vshr.u32	d8, 	d8, #24		\n\t"

		"vmul.u32	d8,	d8, d16		\n\t"

		"vmovl.u8	q9,	d4		\n\t"
		"vmull.u8	q6,	d8,d4		\n\t"
		"vadd.u16	q6,	q6, q9		\n\t"
		"vshrn.u16	d8,	q6, #8		\n\t"
		// Add to 's'
		"vqadd.u8	d0,	d0,d8		\n\t"
		"vst1.32	d0[0],	[%[d]]		\n\t"

		//** Trailing single

	AP"done:\n\t"
//"sub	%[tmp], %[e], #4 \n\t"
//"vmov.i32	d0, $0xffff0000 \n\t"
//"vst1.32	d0[0],	[%[tmp]]		\n\t"


	  : // output regs
	  // Input
          :  [e] "r" (d + l), [d] "r" (d), [s] "r" (s), /*[c] "r" (c),*/
			[tmp] "r" (7)
          : "q0", "q1", "q2","q3", "q4","q5","q6", "q7","q8","memory" // clobbered
   );
#undef AP
#endif
}

static void
_op_blend_pas_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d, int l) {
#ifdef BUILD_NEON_INTRINSICS
   uint16x8_t ad00_16x8;
   uint16x8_t ad01_16x8;
   uint16x8_t ad10_16x8;
   uint16x8_t ad11_16x8;
   uint32x4_t ad0_32x4;
   uint32x4_t ad1_32x4;
   uint32x4_t alpha0_32x4;
   uint32x4_t alpha1_32x4;
   uint32x4_t cond0_32x4;
   uint32x4_t cond1_32x4;
   uint32x4_t d0_32x4;
   uint32x4_t d1_32x4;
   uint32x4_t s0_32x4;
   uint32x4_t s1_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad0_8x16;
   uint8x16_t ad1_8x16;
   uint8x16_t alpha0_8x16;
   uint8x16_t alpha1_8x16;
   uint8x16_t d0_8x16;
   uint8x16_t d1_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad00_8x8;
   uint8x8_t ad01_8x8;
   uint8x8_t ad10_8x8;
   uint8x8_t ad11_8x8;
   uint8x8_t alpha00_8x8;
   uint8x8_t alpha01_8x8;
   uint8x8_t alpha10_8x8;
   uint8x8_t alpha11_8x8;
   uint8x8_t d00_8x8;
   uint8x8_t d01_8x8;
   uint8x8_t d10_8x8;
   uint8x8_t d11_8x8;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);

   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~7);
   while (start < end)
   {
      s0_32x4 = vld1q_u32(s);
      s1_32x4 = vld1q_u32(s+4);

      d0_32x4 = vld1q_u32(start);
      d1_32x4 = vld1q_u32(start+4);

      d0_8x16 = vreinterpretq_u8_u32(d0_32x4);
      d1_8x16 = vreinterpretq_u8_u32(d1_32x4);
      d00_8x8 = vget_low_u8(d0_8x16);
      d01_8x8 = vget_high_u8(d0_8x16);
      d10_8x8 = vget_low_u8(d1_8x16);
      d11_8x8 = vget_high_u8(d1_8x16);

      alpha0_32x4 = vshrq_n_u32(s0_32x4, 24);
      alpha1_32x4 = vshrq_n_u32(s1_32x4, 24);

      alpha0_32x4 = vmulq_u32(x1_32x4, alpha0_32x4);
      alpha1_32x4 = vmulq_u32(x1_32x4, alpha1_32x4);

      alpha0_8x16 = vreinterpretq_u8_u32(alpha0_32x4);
      alpha1_8x16 = vreinterpretq_u8_u32(alpha1_32x4);

      alpha0_8x16 = vsubq_u8(x0_8x16, alpha0_8x16);
      alpha1_8x16 = vsubq_u8(x0_8x16, alpha1_8x16);

      alpha0_32x4 = vreinterpretq_u32_u8(alpha0_8x16);
      alpha1_32x4 = vreinterpretq_u32_u8(alpha1_8x16);

      alpha10_8x8 = vget_low_u8(alpha1_8x16);
      alpha11_8x8 = vget_high_u8(alpha1_8x16);
      alpha00_8x8 = vget_low_u8(alpha0_8x16);
      alpha01_8x8 = vget_high_u8(alpha0_8x16);

      ad00_16x8 = vmull_u8(alpha00_8x8, d00_8x8);
      ad01_16x8 = vmull_u8(alpha01_8x8, d01_8x8);
      ad10_16x8 = vmull_u8(alpha10_8x8, d10_8x8);
      ad11_16x8 = vmull_u8(alpha11_8x8, d11_8x8);
      ad00_8x8 = vshrn_n_u16(ad00_16x8,8);
      ad01_8x8 = vshrn_n_u16(ad01_16x8,8);
      ad10_8x8 = vshrn_n_u16(ad10_16x8,8);
      ad11_8x8 = vshrn_n_u16(ad11_16x8,8);

      ad0_8x16 = vcombine_u8(ad00_8x8, ad01_8x8);
      ad1_8x16 = vcombine_u8(ad10_8x8, ad11_8x8);
      ad0_32x4 = vreinterpretq_u32_u8(ad0_8x16);
      ad1_32x4 = vreinterpretq_u32_u8(ad1_8x16);

      cond0_32x4 = vceqq_u32(alpha0_32x4, x0_32x4);
      cond1_32x4 = vceqq_u32(alpha1_32x4, x0_32x4);

      ad0_32x4 = vbslq_u32(cond0_32x4, d0_32x4, ad0_32x4);
      ad1_32x4 = vbslq_u32(cond1_32x4, d1_32x4, ad1_32x4);

      d0_32x4 = vaddq_u32(s0_32x4, ad0_32x4);
      d1_32x4 = vaddq_u32(s1_32x4, ad1_32x4);

      vst1q_u32(start, d0_32x4);
      vst1q_u32(start+4, d1_32x4);

      s+=8;
      start+=8;
   }
   end += (size & 7);
   while (start <  end)
   {
      int alpha;
      alpha = 256 - (*s >> 24);
      *start = *s++ + MUL_256(alpha, *start);
      start++;
   }
#else
#define AP "blend_pas_dp_"
   DATA32 *e = d + l,*tmp  = e + 32,*pl=(void*)912;
      asm volatile (
        ".fpu neon						\n\t"
		"vmov.i8	q8,	#1			\n\t"
	AP"loopchoose:						\n\t"
		// If aliged - go as fast we can
		"andS	%[tmp], %[d], 	#31			\n\t"
		"beq	"AP"quadstart				\n\t"

		// See if we can at least do our double loop
		"andS	%[tmp], %[d], $0x7			\n\t"
		"beq	"AP"dualstart				\n\t"

	// Ugly single word version
	AP "singleloop:						\n\t"
		"vld1.32	d0[0], [%[s]]!			\n\t"
		"vld1.32	d4[0], [%[d]]			\n\t"

		"vmvn.u8	d8,	d0			\n\t"

		"vshr.u32	d8,	d8,$0x18		\n\t"

		// Mulitply into all fields
		"vmul.u32	d8,	d8, d16			\n\t"

		"vmovl.u8	q9,	d4			\n\t"
		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"
		"vadd.u16	q6,	q6, q9			\n\t"

		"vshrn.u16	d8,	q6, #8			\n\t"

		// Add to s
		"vqadd.u8	d0,	d0,d8			\n\t"
		"vst1.32	d0[0], [%[d]]!			\n\t"

	AP"dualstart:						\n\t"
		"sub		%[tmp], %[e], %[d]		\n\t"
		"cmp		%[tmp], #32			\n\t"
		"blt		"AP"loopout			\n\t"

		// If aligned - go as fast we can
		"andS		%[tmp], %[d], #31 		\n\t"
		"beq		"AP"quadstart			\n\t"


	AP"dualloop:						\n\t"

		"vldm	%[s]!,	{d0)				\n\t"
		"vldm	%[d],	{d4}				\n\t"

		// Subtract from 255 (ie negate) and extract alpha channel
		"vmvn.u8	d8,	d0			\n\t"
		"vshr.u32	d8,	d8,$0x18		\n\t"

		// Mulitply into all fields
		"vmul.u32	d8,	d8, d16			\n\t"

		"vmovl.u8	q9,	d4			\n\t"
		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"
		"vadd.u16	q6,	q6, q9			\n\t"

		"vshrn.u16	d8,	q6, #8			\n\t"

		// Add to s
		"vqadd.u8	d0,	d0,d8			\n\t"
		"vstm		%[d]!,	{d0}			\n\t"

		"andS		%[tmp], %[d], $0x1f		\n\t"
		"bne		"AP"dualloop			\n\t"


        AP"quadstart:						\n\t"
		"sub		%[tmp], %[e], %[d]		\n\t"
		"cmp		%[tmp], #32			\n\t"
		"blt		"AP"loopout			\n\t"

		"sub		%[tmp], %[e],  #31		\n\t"

        AP"quadloop:\n\t"
		"vldm	%[s]!,	{d0,d1,d2,d3)			\n\t"
		"vldm	%[d],	{d4,d5,d6,d7}			\n\t"

		// Subtract from 255 (ie negate) and extract alpha channel
		"vmvn.u8	q4,	q0			\n\t"
			"vmvn.u8	q5,	q1		\n\t"
		"vshr.u32	q4,	q4,$0x18		\n\t"
			"vshr.u32	q5,	q5,$0x18	\n\t"

		// Prepare to preload
		"add	%[pl], %[s], #32			\n\t"

		// Mulitply into all fields
		"vmul.u32	q4,	q4, q8			\n\t"
			"vmul.u32	q5,	q5, q8		\n\t"
		"pld	[%[pl]]					\n\t"

		"vmovl.u8	q9,	d4			\n\t"
		"vmovl.u8	q10,	d5			\n\t"
		"vmovl.u8	q11,	d6			\n\t"
		"vmovl.u8	q12,	d7			\n\t"

		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"
			"vmull.u8	q7,	d10, d6		\n\t"
		"vmull.u8	q2,	d9, d5			\n\t"
			"vmull.u8	q3,	d11, d7		\n\t"

		"vadd.u16	q6,	q6, q9			\n\t"
		"vadd.u16	q2,	q2, q10			\n\t"
		"vadd.u16	q7,	q7, q11			\n\t"
		"vadd.u16	q3,	q3, q12			\n\t"

		"add	%[pl], %[d], #32			\n\t"

		"vshrn.u16	d8,	q6, #8			\n\t"
			"vshrn.u16	d10,	q7, #8		\n\t"
		"vshrn.u16	d9,	q2, #8			\n\t"
			"vshrn.u16	d11,	q3, #8		\n\t"
		"pld	[%[pl]]					\n\t"

		"cmp 		%[tmp], %[pl]			\n\t"
		// Add to s
		"vqadd.u8	q0,	q0,q4			\n\t"
			"vqadd.u8	q1,	q1,q5		\n\t"

		"vstm		%[d]!,	{d0,d1,d2,d3}		\n\t"

		"bhi 		"AP"quadloop			\n\t"

	AP "loopout:						\n\t"
		"cmp 		%[d], %[e]			\n\t"
                "beq 		"AP"done			\n\t"

		"sub		%[tmp],%[e], %[d]		\n\t"
		"cmp		%[tmp],$0x04			\n\t"
		"beq		"AP"singleloop2			\n\t"

		"sub		%[tmp],%[e],$0x7		\n\t"

	AP"dualloop2:						\n\t"
		"vldm		%[s]!,	{d0)			\n\t"
		"vldm		%[d],	{d4}			\n\t"

		// Subtract from 255 (ie negate) and extract alpha channel
		"vmvn.u8	d8,	d0			\n\t"
		"vshr.u32	d8,	d8,$0x18		\n\t"

		// Mulitply into all fields
		"vmul.u32	d8,	d8, d16			\n\t"

		"vmovl.u8	q9,	d4			\n\t"
		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"
		"vadd.u16	q6,	q6, q9			\n\t"

		"vshrn.u16	d8,	q6, #8			\n\t"

		// Add to s
		"vqadd.u8	d0,	d0,d8			\n\t"

		"vstm		%[d]!,	{d0}			\n\t"
		"cmp 		%[tmp], %[d]			\n\t"

		"bhi		"AP"dualloop2			\n\t"

		// Single ??
		"cmp 		%[e], %[d]			\n\t"
		"beq		"AP"done			\n\t"

	AP "singleloop2:					\n\t"
		"vld1.32	d0[0], [%[s]]			\n\t"
		"vld1.32	d4[0], [%[d]]			\n\t"

		"vmvn.u8	d8,	d0			\n\t"

		"vshr.u32	d8,	d8,$0x18		\n\t"

		// Mulitply into all fields
		"vmul.u32	d8,	d8, d16			\n\t"

		// Multiply out
		"vmovl.u8	q9,	d4			\n\t"
		"vmull.u8	q6,	d8, d4			\n\t"
		"vadd.u16	q6,	q6, q9			\n\t"

		"vshrn.u16	d8,	q6, #8			\n\t"

		// Add to s
		"vqadd.u8	d0,	d0,d8			\n\t"

		"vst1.32	d0[0], [%[d]]			\n\t"
	AP "done:\n\t"


         : /* Out */
         : /* In */  [s] "r" (s), [e] "r" (e), [d] "r" (d), [tmp] "r" (tmp),
		[pl] "r" (pl)
         : /* Clobbered */
		 "q0","q1","q2","q3","q4","q5","q6","q7","q8","memory"
      );
#undef AP
#endif
}

#define _op_blend_pan_dp_neon NULL

#define _op_blend_p_dpan_neon _op_blend_p_dp_neon
#define _op_blend_pas_dpan_neon _op_blend_pas_dp_neon
#define _op_blend_pan_dpan_neon _op_blend_pan_dp_neon

static void
init_blend_pixel_span_funcs_neon(void)
{
   op_blend_span_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_blend_p_dp_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_blend_pas_dp_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_blend_pan_dp_neon;

   op_blend_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_p_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_pas_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_pan_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_p_dp_neon(DATA32 s, DATA8 m EINA_UNUSED, DATA32 c, DATA32 *d) {
   c = 256 - (s >> 24);
   *d = s + MUL_256(c, *d);
}


#define _op_blend_pt_pan_dp_neon NULL
#define _op_blend_pt_pas_dp_neon _op_blend_pt_p_dp_neon

#define _op_blend_pt_p_dpan_neon _op_blend_pt_p_dp_neon
#define _op_blend_pt_pan_dpan_neon _op_blend_pt_pan_dp_neon
#define _op_blend_pt_pas_dpan_neon _op_blend_pt_pas_dp_neon

static void
init_blend_pixel_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_blend_pt_p_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_blend_pt_pas_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_blend_pt_pan_dp_neon;

   op_blend_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_p_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_pas_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_pan_dpan_neon;
}
#endif

/*-----*/

/* blend_rel pixel -> dst */

#ifdef BUILD_NEON
static void
_op_blend_rel_p_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t cs0_16x8;
   uint16x8_t cs1_16x8;
   uint16x8_t ld0_16x8;
   uint16x8_t ld1_16x8;
   uint32x4_t c_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t cs_32x4;
   uint32x4_t d_32x4;
   uint32x4_t l_32x4;
   uint32x4_t ld_32x4;
   uint32x4_t s_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t c_8x16;
   uint8x16_t cs_8x16;
   uint8x16_t d_8x16;
   uint8x16_t l_8x16;
   uint8x16_t ld_8x16;
   uint8x16_t s_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t c0_8x8;
   uint8x8_t c1_8x8;
   uint8x8_t cs0_8x8;
   uint8x8_t cs1_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t l0_8x8;
   uint8x8_t l1_8x8;
   uint8x8_t ld0_8x8;
   uint8x8_t ld1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);

   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from d
      d_32x4 = vld1q_u32(d);
      d_8x16 = vreinterpretq_u8_u32(d_32x4);
      d0_8x8 = vget_low_u8(d_8x16);
      d1_8x8 = vget_high_u8(d_8x16);

      // load 4 elements from s
      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);
      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      // calculate l = 256 - (*s >> 24)
      l_32x4 = vshrq_n_u32(s_32x4, 24);
      l_32x4 = vmulq_u32(x1_32x4, l_32x4);
      l_8x16 = vreinterpretq_u8_u32(l_32x4);
      l_8x16 = vsubq_u8(x0_8x16, l_8x16);
      l0_8x8 = vget_low_u8(l_8x16);
      l1_8x8 = vget_high_u8(l_8x16);

      // multiply MUL_256(l, *d)
      ld0_16x8 = vmull_u8(l0_8x8, d0_8x8);
      ld1_16x8 = vmull_u8(l1_8x8, d1_8x8);
      ld0_8x8 = vshrn_n_u16(ld0_16x8,8);
      ld1_8x8 = vshrn_n_u16(ld1_16x8,8);
      ld_8x16 = vcombine_u8(ld0_8x8, ld1_8x8);
      ld_32x4 = vreinterpretq_u32_u8(ld_8x16);

      // select d where l should be 256
      cond_32x4 = vceqq_u32(l_32x4, x0_32x4);
      ld_32x4 = vbslq_u32(cond_32x4, d_32x4, ld_32x4);

      // calculate 1 + (*d >> 24)
      c_32x4 = vshrq_n_u32(d_32x4, 24);
      c_32x4 = vmulq_u32(x1_32x4, c_32x4);
      c_8x16 = vreinterpretq_u8_u32(c_32x4);
      c_8x16 = vaddq_u8(c_8x16, x1_8x16);
      c0_8x8 = vget_low_u8(c_8x16);
      c1_8x8 = vget_high_u8(c_8x16);

      // multiply MUL_256(l, *d)
      cs0_16x8 = vmull_u8(c0_8x8, s0_8x8);
      cs1_16x8 = vmull_u8(c1_8x8, s1_8x8);
      cs0_8x8 = vshrn_n_u16(cs0_16x8,8);
      cs1_8x8 = vshrn_n_u16(cs1_16x8,8);
      cs_8x16 = vcombine_u8(cs0_8x8, cs1_8x8);
      cs_32x4 = vreinterpretq_u32_u8(cs_8x16);

      // select s where c should be 256
      c_32x4 = vreinterpretq_u32_u8(c_8x16);
      cond_32x4 = vceqq_u32(c_32x4, x0_32x4);
      cs_32x4 = vbslq_u32(cond_32x4, s_32x4, cs_32x4);

      // add up everything
      d_32x4 = vaddq_u32(cs_32x4, ld_32x4);

      // save result
      vst1q_u32(d, d_32x4);

      d+=4;
      s+=4;
   }

   end += (l & 3);
   while (d < end)
   {
      l = 256 - (*s >> 24);
      c = 1 + (*d >> 24);
      *d = MUL_256(c, *s) + MUL_256(l, *d);
      d++;
      s++;
   }
}

static void
_op_blend_rel_pan_dp_neon(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   uint16x8_t cs0_16x8;
   uint16x8_t cs1_16x8;
   uint32x4_t c_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t cs_32x4;
   uint32x4_t d_32x4;
   uint32x4_t s_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t c_8x16;
   uint8x16_t cs_8x16;
   uint8x16_t s_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t c0_8x8;
   uint8x8_t c1_8x8;
   uint8x8_t cs0_8x8;
   uint8x8_t cs1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;

   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);
   DATA32 *end = d + (l & ~3);
   while (d < end)
   {
      // load 4 elements from d
      d_32x4 = vld1q_u32(d);

      // load 4 elements from s
      s_32x4 = vld1q_u32(s);
      s_8x16 = vreinterpretq_u8_u32(s_32x4);
      s0_8x8 = vget_low_u8(s_8x16);
      s1_8x8 = vget_high_u8(s_8x16);

      // calculate 1 + (*d >> 24)
      c_32x4 = vshrq_n_u32(d_32x4, 24);
      c_32x4 = vmulq_u32(x1_32x4, c_32x4);
      c_8x16 = vreinterpretq_u8_u32(c_32x4);
      c_8x16 = vaddq_u8(c_8x16, x1_8x16);
      c0_8x8 = vget_low_u8(c_8x16);
      c1_8x8 = vget_high_u8(c_8x16);

      // multiply MUL_256(l, *d)
      cs0_16x8 = vmull_u8(c0_8x8, s0_8x8);
      cs1_16x8 = vmull_u8(c1_8x8, s1_8x8);
      cs0_8x8 = vshrn_n_u16(cs0_16x8,8);
      cs1_8x8 = vshrn_n_u16(cs1_16x8,8);
      cs_8x16 = vcombine_u8(cs0_8x8, cs1_8x8);
      cs_32x4 = vreinterpretq_u32_u8(cs_8x16);

      // select s where c should be 256
      c_32x4 = vreinterpretq_u32_u8(c_8x16);
      cond_32x4 = vceqq_u32(c_32x4, x0_32x4);
      cs_32x4 = vbslq_u32(cond_32x4, s_32x4, cs_32x4);

      // save result
      vst1q_u32(d, cs_32x4);

      d+=4;
      s+=4;
   }

   end += (l & 3);
   while (d < end)
   {
      c = 1 + (*d >> 24);
      *d++ = MUL_256(c, *s);
      s++;
   }
}

#define _op_blend_rel_pas_dp_neon _op_blend_rel_p_dp_neon

#define _op_blend_rel_p_dpan_neon _op_blend_p_dpan_neon
#define _op_blend_rel_pan_dpan_neon _op_blend_pan_dpan_neon
#define _op_blend_rel_pas_dpan_neon _op_blend_pas_dpan_neon

static void
init_blend_rel_pixel_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_blend_rel_p_dp_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_blend_rel_pas_dp_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_blend_rel_pan_dp_neon;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_p_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pas_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pan_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_rel_pt_p_dp_neon(DATA32 s, DATA8 m EINA_UNUSED, DATA32 c, DATA32 *d) {
   c = 256 - (s >> 24);
   *d = MUL_SYM(*d >> 24, s) + MUL_256(c, *d);
}

#define _op_blend_rel_pt_pas_dp_neon _op_blend_rel_pt_p_dp_neon
#define _op_blend_rel_pt_pan_dp_neon _op_blend_rel_pt_p_dp_neon

#define _op_blend_rel_pt_p_dpan_neon _op_blend_pt_p_dpan_neon
#define _op_blend_rel_pt_pas_dpan_neon _op_blend_pt_pas_dpan_neon
#define _op_blend_rel_pt_pan_dpan_neon _op_blend_pt_pan_dpan_neon

static void
init_blend_rel_pixel_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_blend_rel_pt_p_dp_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_blend_rel_pt_pas_dp_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_blend_rel_pt_pan_dp_neon;

   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_dpan_neon;
}
#endif
