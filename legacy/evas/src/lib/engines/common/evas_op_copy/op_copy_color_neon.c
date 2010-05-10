
/* copy color --> dst */

#ifdef BUILD_NEON
static void
_op_copy_c_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
#define AP "COPY_C_DP_"
   uint32_t *e = d + l,*tmp;
   asm volatile (

		"vdup.i32 	q0,	%[c]		\n\t"

		// Can we do 32 byte?
		"andS		%[tmp],	%[d], $0x1f	\n\t"
		"beq		"AP"quadstart		\n\t"

		// Can we do at least 16 byte?
		"andS		%[tmp], %[d], $0x4	\n\t"
		"beq		"AP"dualstart		\n\t"

	// Only once
	AP"singleloop:					\n\t"
		"vst1.32	d0[0],  [%[d]]		\n\t"
		"add		%[d], #4		\n\t"

	// Up to 3 times
	AP"dualstart:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"blt		"AP"loopout		\n\t"

	AP"dualloop:					\n\t"
		"vstr.32	d0, [%[d]]		\n\t"

		"add		%[d], #8		\n\t"
		"andS		%[tmp], %[d], $0x1f	\n\t"
		"bne		"AP"dualloop		\n\t"


	AP"quadstart:					\n\t"
		"sub		%[tmp], %[e], %[d]	\n\t"
		"cmp		%[tmp], #32		\n\t"
		"blt		"AP"loopout		\n\t"

		"vmov		q1, q0			\n\t"
		"sub		%[tmp],%[e],#31		\n\t"

	AP "quadloop:					\n\t"
		"vstm		%[d]!,	{d0,d1,d2,d3}	\n\t"

		"cmp		%[tmp], %[d]		\n\t"
                "bhi		"AP"quadloop		\n\t"


	AP "loopout:					\n\t"
		"cmp 		%[d], %[e]		\n\t"
                "beq 		"AP"done		\n\t"
		"sub		%[tmp],%[e], %[d]	\n\t"
		"cmp		%[tmp],$0x04		\n\t"
		"beq		"AP"singleloop2		\n\t"

	AP "dualloop2:					\n\t"
		"sub		%[tmp],%[e],#7		\n\t"
	AP "dualloop2int:				\n\t"
		"vstr.64	d0, [%[d]]		\n\t"

		"add		%[d], #8		\n\t"
		"cmp 		%[tmp], %[d]		\n\t"
		"bhi 		"AP"dualloop2int	\n\t"

		// Single ??
		"cmp 		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP "singleloop2:				\n\t"
		"vst1.32	d0[0], [%[d]]		\n\t"

	AP "done:\n\t"

		: // No output regs
		// Input
		: [c] "r" (c), [e] "r" (e), [d] "r" (d),[tmp] "r" (tmp)
		// Clobbered
		: "q0","q1","memory"


   );
}

#define _op_copy_cn_dp_neon _op_copy_c_dp_neon
#define _op_copy_can_dp_neon _op_copy_c_dp_neon
#define _op_copy_caa_dp_neon _op_copy_c_dp_neon

#define _op_copy_cn_dpan_neon _op_copy_c_dp_neon
#define _op_copy_c_dpan_neon _op_copy_c_dp_neon
#define _op_copy_can_dpan_neon _op_copy_c_dp_neon
#define _op_copy_caa_dpan_neon _op_copy_c_dp_neon

static void
init_copy_color_span_funcs_neon(void)
{
   op_copy_span_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_cn_dp_neon;
   op_copy_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_c_dp_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_can_dp_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_caa_dp_neon;

   op_copy_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_cn_dpan_neon;
   op_copy_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_c_dpan_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_can_dpan_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_c_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   *d = c;
}

#define _op_copy_pt_cn_dp_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_can_dp_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_caa_dp_neon _op_copy_pt_c_dp_neon

#define _op_copy_pt_cn_dpan_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_c_dpan_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_can_dpan_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_caa_dpan_neon _op_copy_pt_c_dp_neon

static void
init_copy_color_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_cn_dp_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_pt_c_dp_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_pt_can_dp_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_pt_caa_dp_neon;

   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_cn_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_pt_c_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_pt_can_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_pt_caa_dpan_neon;
}
#endif

/*-----*/

/* copy_rel color --> dst */

#ifdef BUILD_NEON
static void
_op_copy_rel_c_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e = d + l;
   for (; d < e; d++) {
	*d = MUL_SYM(*d >> 24, c);
   }
}

#define _op_copy_rel_cn_dp_neon _op_copy_rel_c_dp_neon
#define _op_copy_rel_can_dp_neon _op_copy_rel_c_dp_neon
#define _op_copy_rel_caa_dp_neon _op_copy_rel_c_dp_neon

#define _op_copy_rel_cn_dpan_neon _op_copy_cn_dpan_neon
#define _op_copy_rel_c_dpan_neon _op_copy_c_dpan_neon
#define _op_copy_rel_can_dpan_neon _op_copy_can_dpan_neon
#define _op_copy_rel_caa_dpan_neon _op_copy_caa_dpan_neon

static void
init_copy_rel_color_span_funcs_neon(void)
{
   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_cn_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_c_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_can_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_caa_dp_neon;

   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_cn_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_c_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_can_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_rel_pt_c_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   s = 1 + (*d >> 24);
   *d = MUL_256(s, c);
}


#define _op_copy_rel_pt_cn_dp_neon _op_copy_rel_pt_c_dp_neon
#define _op_copy_rel_pt_can_dp_neon _op_copy_rel_pt_c_dp_neon
#define _op_copy_rel_pt_caa_dp_neon _op_copy_rel_pt_c_dp_neon

#define _op_copy_rel_pt_cn_dpan_neon _op_copy_pt_cn_dpan_neon
#define _op_copy_rel_pt_c_dpan_neon _op_copy_pt_c_dpan_neon
#define _op_copy_rel_pt_can_dpan_neon _op_copy_pt_can_dpan_neon
#define _op_copy_rel_pt_caa_dpan_neon _op_copy_pt_caa_dpan_neon

static void
init_copy_rel_color_pt_funcs_neon(void)
{
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_cn_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_pt_c_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_pt_can_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_pt_caa_dp_neon;

   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_cn_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_pt_c_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pt_can_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pt_caa_dpan_neon;
}
#endif
