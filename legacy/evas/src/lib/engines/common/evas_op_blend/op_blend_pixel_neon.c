
/* blend pixel --> dst */

#ifdef BUILD_NEON
static void
_op_blend_p_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
#define AP "blend_p_dp_"
  asm volatile (
	//** init
	"vmov.i8	q8,	$0x1		\n\t"

	AP "loopchoose:					\n\t"
		// If aligned already - straight to octs
		"andS		%[tmp], %[d],$0x1f		\n\t"
		"beq		"AP"octloops			\n\t"

		"andS		%[tmp], %[d],$0xf		\n\t"
		"beq		"AP"quadloops			\n\t"

		"andS		%[tmp], %[d],$0x4		\n\t"
		"beq		"AP"dualloop			\n\t"

	// Only ever executes once, fall through to dual
	AP "singleloop:					\n\t"
		"vld1.32	d0[0],	[%[s]]!		\n\t"
		"vld1.32	d4[0],  [%[d]]		\n\t"

		"vmvn.u8	d8, 	d0		\n\t"
		"vshr.u32	d8, 	d8, #24		\n\t"

		"vmul.u32	d8,	d16, d8		\n\t"

		"vmull.u8	q6,	d4,d8		\n\t"
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

		"vmull.u8	q6,	d4,d8		\n\t"
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

		// a * d  (clobbering 'd'/q7)
		"vmull.u8	q6,	d4,d8		\n\t"
		"vmull.u8	q2,	d5,d9		\n\t"

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

		"sub		%[tmp],%[e],#64	\n\t"


	AP "octloopint:\n\t"
		//** Oct loop
		"vldm	%[s]!,	{d0,d1,d2,d3)		\n\t"
		"vldm	%[d],	{d4,d5,d6,d7}		\n\t"


		// Copy s.a into q2 (>> 24) & subtract from 255
		"vmvn.u8	q4,	q0		\n\t"
			"vmvn.u8	q5,	q1	\n\t"
		"vshr.u32	q4,	q4,$0x18	\n\t"
			"vshr.u32	q5,	q5,$0x18\n\t"

		// Multiply into all fields
		"vmul.u32	q4,	q8,q4		\n\t"
			"vmul.u32	q5,	q8,q5	\n\t"


		// a * d  (clobbering 'd'/q7)
		"vmull.u8	q6,	d4,d8		\n\t"
		"vmull.u8	q2,	d5,d9		\n\t"
			"vmull.u8	q7,	d6,d10	\n\t"
			"vmull.u8	q3,	d7,d11	\n\t"

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
//"sub %[tmp], %[d], #4\n\t"
//"vmov.i16	d0, $0xff00 \n\t"
//"vst1.32	d0[0],	[%[tmp]]		\n\t"

		"cmp 		%[d], %[e]\n\t"
                "beq 		"AP"done\n\t"
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

		"vmull.u8	q6,	d4,d8		\n\t"
		"vshrn.u16	d8,	q6, #8		\n\t"
		// Add to 's'
		"vqadd.u8	d4,	d8,d0		\n\t"

		"vstr.32	d4,	[%[d]]		\n\t"
		"add		%[d],   #8		\n\t"

		"cmp 		%[tmp], %[d]		\n\t"
		"bhi 		"AP"dualloop2int		\n\t"

		// Single ??
		"cmp 		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP"singleloop2:					\n\t"
		"vld1.32	d0[0],	[%[s]]		\n\t"
		"vld1.32	d4[0],  [%[d]]		\n\t"

		"vmvn.u8	d8, 	d0		\n\t"
		"vshr.u32	d8, 	d8, #24		\n\t"

		"vmul.u32	d8,	d8, d16		\n\t"

		"vmull.u8	q6,	d8,d4		\n\t"
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
          :  [e] "r" (d + l), [d] "r" (d), [s] "r" (s), [c] "r" (c),
			[tmp] "r" (7)
          : "q0", "q1", "q2","q3", "q4","q5","q6", "q7","q8","memory" // clobbered
   );
#undef AP

}

static void
_op_blend_pas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
#define AP "blend_pas_dp_"
   DATA32 *e = d + l,*tmp  = e + 32,*pl=(void*)912;
      asm volatile (
		"vmov.i8	q8,	#1			\n\t"
	AP"loopchoose:						\n\t"
		// If aliged - go as fast we can
		"andS	%[tmp], %[d], 	#31		\n\t"
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

		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"

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

		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"

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
		"add	%[pl], %[s], #32\n\t"

		// Mulitply into all fields
		"vmul.u32	q4,	q4, q8			\n\t"
			"vmul.u32	q5,	q5, q8		\n\t"
		"pld	[%[pl]]\n\t"

		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"
			"vmull.u8	q7,	d10, d6		\n\t"
		"vmull.u8	q2,	d9, d5			\n\t"
			"vmull.u8	q3,	d11, d7		\n\t"

		"add	%[pl], %[d], #32\n\t"

		"vshrn.u16	d8,	q6, #8			\n\t"
			"vshrn.u16	d10,	q7, #8		\n\t"
		"vshrn.u16	d9,	q2, #8			\n\t"
			"vshrn.u16	d11,	q3, #8		\n\t"
		"pld	[%[pl]]\n\t"

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

		"sub		%[tmp],%[e],$0x7	\n\t"

	AP"dualloop2:						\n\t"
		"vldm	%[s]!,	{d0)				\n\t"
		"vldm	%[d],	{d4}				\n\t"

		// Subtract from 255 (ie negate) and extract alpha channel
		"vmvn.u8	d8,	d0			\n\t"
		"vshr.u32	d8,	d8,$0x18		\n\t"

		// Mulitply into all fields
		"vmul.u32	d8,	d8, d16			\n\t"

		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"

		"vshrn.u16	d8,	q6, #8			\n\t"

		// Add to s
		"vqadd.u8	d0,	d0,d8			\n\t"

		"vstm		%[d]!,	{d0}			\n\t"
		"cmp 		%[tmp], %[d]			\n\t"

		"bhi		"AP"dualloop2			\n\t"

		// Single ??
		"cmp 		%[e], %[d]		\n\t"
		"beq		"AP"done		\n\t"

	AP "singleloop2:					\n\t"
		"vld1.32	d0[0], [%[s]]			\n\t"
		"vld1.32	d4[0], [%[d]]			\n\t"

		"vmvn.u8	d8,	d0			\n\t"

		"vshr.u32	d8,	d8,$0x18		\n\t"

		// Mulitply into all fields
		"vmul.u32	d8,	d8, d16			\n\t"

		// Multiply out
		"vmull.u8	q6,	d8, d4			\n\t"

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
_op_blend_pt_p_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
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
_op_blend_rel_p_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
      l = 256 - (*s >> 24);
      c = 1 + (*d >> 24);
      *d = MUL_256(c, *s) + MUL_256(l, *d);
      d++;
      s++;
   }
}

static void
_op_blend_rel_pan_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
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
_op_blend_rel_pt_p_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
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
