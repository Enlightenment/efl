
/* blend color --> dst */

#ifdef BUILD_NEON
static void
_op_blend_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
	DATA32 *e, *tmp = 0;
#define AP	"B_C_DP"
   asm volatile (
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
		"vmull.u8	q0, d0, d14		\n\t"
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
		"vmull.u8	q1, d0, d14		\n\t"
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

		"vmull.u8	q2, d0, d14		\n\t"
		"vmull.u8	q3, d1, d15		\n\t"
		"vmull.u8	q4, d2, d14		\n\t"
		"vmull.u8	q5, d3, d15		\n\t"

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
		"vmull.u8	q1, d0, d14		\n\t"
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
		"vmull.u8	q1, d0, d14		\n\t"
		"vshrn.u16	d0, q1, #8		\n\t"
		"vqadd.u8	d0, d0, d12		\n\t"

		"vst1.32	d0[0], [%[d]]		\n\t"

	AP "done:\n\t"

	  : // output regs
	  // Input
          :  [e] "r" (e = d + l), [d] "r" (d), [c] "r" (c), [tmp] "r" (tmp)
          : "q0", "q1", "q2","q3", "q4","q5","q6", "q7","memory" // clobbered

	);
#undef AP

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
_op_blend_pt_c_dp_neon(DATA32 s __UNUSED__, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
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
_op_blend_rel_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha = 256 - (c >> 24);
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = MUL_SYM(*d >> 24, c) + MUL_256(alpha, *d);
                        d++;
                     });
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
_op_blend_rel_pt_c_dp_neon(DATA32 s __UNUSED__, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
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
