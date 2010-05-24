
/* blend mask x color -> dst */

#ifdef BUILD_NEON
static void
_op_blend_mas_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha = 256 - (c >> 24);
#define AP "blend_mas_c_dp_"
     asm volatile (
	"	vdup.i32	q15, %[c]			\n\t"
	"	vmov.i8		q14,	#1			\n\t"

		// If aligned already - straight to quads
	"	andS		%[tmp], %[d],$0xf		\n\t"
	"	beq		"AP"quadloops			\n\t"

	"	andS		%[tmp], %[d],$0x4		\n\t"
	"	beq		"AP"dualloop			\n\t"

	AP"singleloop:						\n\t"
	"	vld1.8		d0[0],	[%[m]]!			\n\t"
	"	vld1.32		d4[0],	[%[d]]			\n\t"
	"	vdup.u8		d0,	d0[0]			\n\t"
	"	vmull.u8	q4,	d0, d30			\n\t"
	"	vshrn.u16	d12,	q4, #8			\n\t"
	"	vmvn.u16	d14,	d12			\n\t"
	"	vshr.u32	d16,	d14, #24		\n\t"
	"	vmul.u32	d16,	d16, d28		\n\t"
	"	vmull.u8	q7,	d16, d4			\n\t"
	"	vshrn.u16	d0,	q7, #8			\n\t"
	"	vqadd.u8	d0, 	d0, d12			\n\t"
	"	vst1.32		d0[0],	[%[d]]!			\n\t"

		// Can we go the fast path?
	"	andS		%[tmp], %[d],$0xf		\n\t"
	"	beq		"AP"quadloops			\n\t"

	AP"dualloop:						\n\t"
	"	sub		%[tmp], %[e], %[d]		\n\t"
	"	cmp		%[tmp], #16			\n\t"
	"	blt		"AP"loopout			\n\t"

	"	vld1.16		d0[0],	[%[m]]!			\n\t"
	"	vldm		%[d],	{d4}			\n\t"
	"	vmovl.u8	q0,	d0			\n\t"
	"	vmovl.u8	q0,	d0			\n\t"
	"	vmul.u32	q0,	q14			\n\t"
	"	vmull.u8	q4,	d0, d30			\n\t"
	"	vshrn.u16	d12,	q4, #8			\n\t"
	"	vmvn.u16	d14,	d12			\n\t"
	"	vshr.u32	d16,	d14, #24		\n\t"
	"	vmul.u32	d16,	d16, d28		\n\t"
	"	vmull.u8	q7,	d16, d4			\n\t"
	"	vshrn.u16	d0,	q7, #8			\n\t"
	"	vqadd.u8	q0, 	q0, q6			\n\t"
	"	vstm		%[d]!,	{d0}			\n\t"

	AP"quadloops:						\n\t"
	"	sub		%[tmp], %[e], %[d]		\n\t"
	"	cmp		%[tmp], #16			\n\t"
	"	blt		"AP"loopout			\n\t"
	"	sub		%[tmp], %[e], #15		\n\t"

	"	sub		%[d],	#16			\n\t"
	AP"fastloop:"
	"	add		%[d],	#16			\n\t"
	"	cmp		%[tmp], %[d]			\n\t"
	"	ble		"AP"loopout			\n\t"
	AP"quadloopint:						\n\t"
	"	ldr		%[x],	[%[m]]			\n\t"
	"	add 		%[m], #4			\n\t"
	"	cmp		%[x],	#0			\n\t"
	"	beq		"AP"fastloop			\n\t"
	"	vmov.32		d0[0],	%[x]			\n\t"
	"	vldm		%[d], {d4,d5}			\n\t"

	// Expand M: Fixme: Can we do this quicker?
	"	vmovl.u8	q0,	d0			\n\t"
	"	vmovl.u8	q0,	d0			\n\t"
	"	vmul.u32	q0,	q14			\n\t"

	// Multiply	a * c
	"	vmull.u8	q4,	d0, d30			\n\t"
	"	vmull.u8	q5,	d1, d31			\n\t"

	// Shorten
	"	vshrn.u16	d12,	q4, #8			\n\t"
	"	vshrn.u16	d13,	q5, #8			\n\t"

	// extract negated alpha
	"	vmvn.u16	q7,	q6			\n\t"
	"	vshr.u32	q8,	q7, #24			\n\t"
	"	vmul.u32	q8,	q8, q14			\n\t"

	// Multiply
	"	vmull.u8	q7,	d16, d4			\n\t"
	"	vmull.u8	q8,	d17, d5			\n\t"

	"	vshrn.u16	d0,	q7, #8			\n\t"
	"	vshrn.u16	d1,	q8, #8			\n\t"

	// Add
	"	vqadd.u8	q0, 	q0, q6			\n\t"

	"	vstm		%[d]!,	{d0,d1}			\n\t"

	"	cmp		%[tmp], %[d]			\n\t"
	"	bhi		"AP"quadloopint			\n\t"

	AP"loopout:						\n\t"
	"	cmp		%[d], %[e]			\n\t"
	"	beq		"AP"done			\n\t"
	"	sub		%[tmp],%[e], %[d]		\n\t"
	"	cmp		%[tmp],#4			\n\t"
	"	beq		"AP"singleout			\n\t"

	AP "dualloop2:					\n\t"
		"sub		%[tmp],%[e],$0x7	\n\t"
	"	vld1.16		d0[0],	[%[m]]!			\n\t"
	"	vldm		%[d],	{d4}			\n\t"
	"	vmovl.u8	q0,	d0			\n\t"
	"	vmovl.u8	q0,	d0			\n\t"
	"	vmul.u32	q0,	q14			\n\t"
	"	vmull.u8	q4,	d0, d30			\n\t"
	"	vshrn.u16	d12,	q4, #8			\n\t"
	"	vmvn.u16	d14,	d12			\n\t"
	"	vshr.u32	d16,	d14, #24		\n\t"
	"	vmul.u32	d16,	d16, d28		\n\t"
	"	vmull.u8	q7,	d16, d4			\n\t"
	"	vshrn.u16	d0,	q7, #8			\n\t"
	"	vqadd.u8	q0, 	q0, q6			\n\t"
	"	vstm		%[d]!,	{d0}			\n\t"

	"	cmp 		%[e], %[d]		\n\t"
	"	beq		"AP"done		\n\t"

	AP"singleout:						\n\t"
	"	vld1.8		d0[0],	[%[m]]!			\n\t"
	"	vld1.32		d4[0],	[%[d]]			\n\t"
	"	vdup.u8		d0,	d0[0]			\n\t"
	"	vmull.u8	q4,	d0, d30			\n\t"
	"	vshrn.u16	d12,	q4, #8			\n\t"
	"	vmvn.u16	d14,	d12			\n\t"
	"	vshr.u32	d16,	d14, #24		\n\t"
	"	vmul.u32	d16,	d16, d28		\n\t"
	"	vmull.u8	q7,	d16, d4			\n\t"
	"	vshrn.u16	d0,	q7, #8			\n\t"
	"	vqadd.u8	q0, 	q0, q6			\n\t"
	"	vst1.32		d0[0],	[%[d]]!			\n\t"

	AP"done:						\n\t"

	: // Out
	:  [e] "r" (d + l), [d] "r" (d), [c] "r" (c),
		[tmp] "r" (7), [m] "r" (m), [x] "r" (0)
          : "q0", "q1", "q2","q3", "q4","q5","q6", "q7","q8","q14","q15",
			"memory" // clobbered
	);
#undef AP
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_mas_can_dp_neon(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e,*tmp;
   int alpha;
#define AP	"_blend_mas_can_dp_neon_"
     asm volatile (
		"vdup.u32	q9,	%[c]		\n\t"
		"vmov.i8	q15,	#1		\n\t"
		"vmov.i8	q14,	#0		\n\t"

		// Make C 16 bit (C in q3/q2)
		"vmovl.u8	q3,	d19		\n\t"
		"vmovl.u8	q2,	d18		\n\t"

		// Which loop to start
	"	andS		%[tmp], %[d],$0xf	\n\t"
	"	beq		"AP"quadloop		\n\t"

	"	andS		%[tmp], %[d], #4	\n\t"
	"	beq		"AP"dualloop		\n\t"


	AP"singleloop:					\n\t"
	"	vld1.8		d0[0],	[%[m]]!		\n\t"
	"	vld1.32		d8[0],  [%[d]]		\n\t"
	"	vdup.u8		d0,	d0[0]		\n\t"
	"	vshr.u8		d0,	d0, 	#1	\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmovl.u8	q4,	d8		\n\t"
	"	vsub.s16	q6,	q2,	q4	\n\t"
	"	vmul.s16	q6,	q0		\n\t"
	"	vshr.s16	q6,	#7		\n\t"
	"	vadd.s16	q6,	q4		\n\t"
	"	vqmovun.s16	d2, 	q6		\n\t"
	"	vst1.32		d2[0],	[%[d]]!		\n\t"

	"	andS		%[tmp], %[d],	#15	\n\t"
	"	beq		"AP"quadloop		\n\t"

	AP"dualloop:					\n\t"
	"	vld1.16	d0[0],	[%[m]]!		\n\t"
	"	vldm		%[d],		{d8}	\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmul.u32	d0,	d0,	d30	\n\t"
	"	vshr.u8	d0,	d0, #1		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmovl.u8	q4,	d8		\n\t"
	"	vsub.s16	q6,	q2, q4		\n\t"
	"	vmul.s16	q6,	q0		\n\t"
	"	vshr.s16	q6,  #7			\n\t"
	"	vadd.s16	q6,  q4			\n\t"
	"	vqmovun.s16	d2,  q6			\n\t"
	"	vstm		%[d]!,	{d2}	\n\t"

	AP"quadloop:					\n\t"
	"	sub		%[tmp], %[e], %[d]	\n\t"
	"	cmp		%[tmp], #16		\n\t"
	"	blt		"AP"loopout		\n\t"
	"	sub		%[tmp], %[e], #15	\n\t"

	"	sub		%[d],	#16		\n\t"
	AP"fastloop:					\n\t"
	"	add		%[d],	#16		\n\t"
	"	cmp		%[tmp], %[d]		\n\t"
	"	ble		"AP"loopout		\n\t"

	AP"quadloopint:					\n\t"
		// Load the mask: 4 bytes: It has d0/d1
	"	ldr		%[x],	[%[m]]		\n\t"
	"	add		%[m], #4		\n\t"
	"	cmp		%[x],	#0		\n\t"
	"	beq		"AP"fastloop		\n\t"
	"	vmov.32		d0[0],	%[x]		\n\t"

		// Load d into d8/d9 q4
	"	vldm		%[d],	{d8,d9}		\n\t"
	"	cmp		%[x],	$0xffffffff	\n\t"
	"	beq		"AP"quadstore		\n\t"


		// Get the alpha channel ready (m)
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmul.u32	q0,	q0,q15		\n\t"
		// Lop a bit off to prevent overflow
	"	vshr.u8	q0,	q0, #1		\n\t"

		// Now make it 16 bit
	"	vmovl.u8	q1,	d1		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"

		// 16 bit 'd'
	"	vmovl.u8	q5,	d9		\n\t"
	"	vmovl.u8	q4,	d8		\n\t"

		// Diff 'd' & 'c'
	"	vsub.s16	q7,	q3, q5		\n\t"
	"	vsub.s16	q6,	q2, q4		\n\t"

	"	vmul.s16	q7,	q1		\n\t"
	"	vmul.s16	q6,	q0		\n\t"

		// Shift results a bit
	"	vshr.s16	q7,  #7			\n\t"
	"	vshr.s16	q6,  #7			\n\t"

		// Add 'd'
	"	vadd.s16	q7,  q5			\n\t"
	"	vadd.s16	q6,  q4			\n\t"

		// Make sure none are negative
	"	vqmovun.s16	d9,  q7			\n\t"
	"	vqmovun.s16	d8,  q6			\n\t"

	"	vstm		%[d]!,	{d8,d9}	\n\t"

	"	cmp		%[tmp], %[d]		\n\t"
	"	bhi		"AP"quadloopint		\n\t"
	"	b		"AP"loopout		\n\t"

	AP"quadstore:					\n\t"
	"	vstm		%[d]!,	{d18,d19}	\n\t"
	"	cmp		%[tmp], %[d]		\n\t"
	"	bhi		"AP"quadloopint		\n\t"


	AP"loopout:					\n\t"

	"	cmp		%[e], %[d]		\n\t"
	"	beq		"AP"done		\n\t"

	"	sub		%[tmp],%[e], %[d]	\n\t"
	"	cmp		%[tmp],#8		\n\t"

	"	blt		"AP"onebyte		\n\t"

		// Load the mask: 2 bytes: It has d0
	"	vld1.16	d0[0],	[%[m]]!		\n\t"

		// Load d into d8/d9 q4
	"	vldm		%[d],		{d8}	\n\t"

		// Get the alpha channel ready (m)
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmul.u32	d0,	d0,	d30	\n\t"
		// Lop a bit off to prevent overflow
	"	vshr.u8	d0,	d0, #1		\n\t"

		// Now make it 16 bit
	"	vmovl.u8	q0,	d0		\n\t"

		// 16 bit 'd'
	"	vmovl.u8	q4,	d8		\n\t"

		// Diff 'd' & 'c'
	"	vsub.s16	q6,	q2, q4		\n\t"

	"	vmul.s16	q6,	q0		\n\t"

		// Shift results a bit
	"	vshr.s16	q6,  #7			\n\t"

		// Add 'd'
		"vadd.s16	q6,  q4			\n\t"

		// Make sure none are negative
		"vqmovun.s16	d2,  q6			\n\t"

		"vstm		%[d]!,	{d2}	\n\t"

		"cmp		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP"onebyte:					\n\t"
		"vld1.8	d0[0],	[%[m]]!			\n\t"
		"vld1.32	d8[0],	[%[d]]		\n\t"
		"vdup.u8	d0,	d0[0]		\n\t"
		"vshr.u8	d0,	d0, #1		\n\t"
		"vmovl.u8	q0,	d0		\n\t"
		"vmovl.u8	q4,	d8		\n\t"
		"vsub.s16	q6,	q2, q4		\n\t"
		"vmul.s16	q6,	q0		\n\t"
		"vshr.s16	q6,  #7			\n\t"
		"vadd.s16	q6,  q4			\n\t"
		"vqmovun.s16	d2,  q6			\n\t"
		"vst1.32	d2[0], [%[d]]!		\n\t"

	AP"done:					\n\t"

	  : // output regs
	  // Input
          :  [e] "r" (e = d + l), [d] "r" (d), [c] "r" (c),
		[m] "r" (m), [tmp] "r" (7), [x] "r" (33)
          : "q0", "q1", "q2","q3", "q4","q5","q6", "q7","q14","q15",
			"memory" // clobbered

     );
#undef AP
}
#endif

#ifdef BUILD_NEON
#define _op_blend_mas_cn_dp_neon _op_blend_mas_can_dp_neon
#define _op_blend_mas_caa_dp_neon _op_blend_mas_c_dp_neon

#define _op_blend_mas_c_dpan_neon _op_blend_mas_c_dp_neon
#define _op_blend_mas_cn_dpan_neon _op_blend_mas_cn_dp_neon
#define _op_blend_mas_can_dpan_neon _op_blend_mas_can_dp_neon
#define _op_blend_mas_caa_dpan_neon _op_blend_mas_caa_dp_neon

static void
init_blend_mask_color_span_funcs_neon(void)
{
   op_blend_span_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_blend_mas_c_dp_neon;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_mas_cn_dp_neon;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_blend_mas_can_dp_neon;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_blend_mas_caa_dp_neon;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_blend_mas_c_dpan_neon;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_mas_cn_dpan_neon;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_blend_mas_can_dpan_neon;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_blend_mas_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_mas_c_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   s = MUL_SYM(m, c);
   c = 256 - (s >> 24);
   *d = MUL_SYM(*d >> 24, s) + MUL_256(c, *d);
}


#define _op_blend_pt_mas_cn_dp_neon _op_blend_pt_mas_c_dp_neon
#define _op_blend_pt_mas_can_dp_neon _op_blend_pt_mas_c_dp_neon
#define _op_blend_pt_mas_caa_dp_neon _op_blend_pt_mas_c_dp_neon

#define _op_blend_pt_mas_c_dpan_neon _op_blend_pt_mas_c_dp_neon
#define _op_blend_pt_mas_cn_dpan_neon _op_blend_pt_mas_cn_dp_neon
#define _op_blend_pt_mas_can_dpan_neon _op_blend_pt_mas_can_dp_neon
#define _op_blend_pt_mas_caa_dpan_neon _op_blend_pt_mas_caa_dp_neon

static void
init_blend_mask_color_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_blend_pt_mas_c_dp_neon;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pt_mas_cn_dp_neon;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_blend_pt_mas_can_dp_neon;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_blend_pt_mas_caa_dp_neon;

   op_blend_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_blend_pt_mas_c_dpan_neon;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_mas_cn_dpan_neon;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_blend_pt_mas_can_dpan_neon;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_mas_caa_dpan_neon;
}
#endif

/*-----*/

/* blend_rel mask x color -> dst */

#ifdef BUILD_NEON
static void
_op_blend_rel_mas_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        DATA32 mc = MUL_SYM(*m, c);
                        alpha = 256 - (mc >> 24);
                        *d = MUL_SYM(*d >> 24, mc) + MUL_256(alpha, *d);
                        d++;
                        m++;
                     });
}

#define _op_blend_rel_mas_cn_dp_neon _op_blend_rel_mas_c_dp_neon
#define _op_blend_rel_mas_can_dp_neon _op_blend_rel_mas_c_dp_neon
#define _op_blend_rel_mas_caa_dp_neon _op_blend_rel_mas_c_dp_neon

#define _op_blend_rel_mas_c_dpan_neon _op_blend_mas_c_dpan_neon
#define _op_blend_rel_mas_cn_dpan_neon _op_blend_mas_cn_dpan_neon
#define _op_blend_rel_mas_can_dpan_neon _op_blend_mas_can_dpan_neon
#define _op_blend_rel_mas_caa_dpan_neon _op_blend_mas_caa_dpan_neon

static void
init_blend_rel_mask_color_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_blend_rel_mas_c_dp_neon;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_rel_mas_cn_dp_neon;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_blend_rel_mas_can_dp_neon;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_blend_rel_mas_caa_dp_neon;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_blend_rel_mas_c_dpan_neon;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_mas_cn_dpan_neon;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_mas_can_dpan_neon;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_mas_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_rel_pt_mas_c_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   s = MUL_SYM(m, c);
   c = 256 - (s >> 24);
   *d = MUL_SYM(*d >> 24, s) + MUL_256(c, *d);
}

#define _op_blend_rel_pt_mas_cn_dp_neon _op_blend_rel_pt_mas_c_dp_neon
#define _op_blend_rel_pt_mas_can_dp_neon _op_blend_rel_pt_mas_c_dp_neon
#define _op_blend_rel_pt_mas_caa_dp_neon _op_blend_rel_pt_mas_c_dp_neon

#define _op_blend_rel_pt_mas_c_dpan_neon _op_blend_pt_mas_c_dpan_neon
#define _op_blend_rel_pt_mas_cn_dpan_neon _op_blend_pt_mas_cn_dpan_neon
#define _op_blend_rel_pt_mas_can_dpan_neon _op_blend_pt_mas_can_dpan_neon
#define _op_blend_rel_pt_mas_caa_dpan_neon _op_blend_pt_mas_caa_dpan_neon

static void
init_blend_rel_mask_color_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_blend_rel_pt_mas_c_dp_neon;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_rel_pt_mas_cn_dp_neon;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_blend_rel_pt_mas_can_dp_neon;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_blend_rel_pt_mas_caa_dp_neon;

   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_mas_c_dpan_neon;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_mas_cn_dpan_neon;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pt_mas_can_dpan_neon;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_mas_caa_dpan_neon;
}
#endif
