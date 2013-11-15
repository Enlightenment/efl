#define NEONDEBUG 0


#if NEONDEBUG
#define DEBUG_FNCOUNT(x)	\
	do {			\
	static int _foo = 0;		\
	if (_foo++%10000 ==0)		\
		printf("%s %+d %s: %d (%s)\n",__FILE__,__LINE__,__FUNCTION__,\
				_foo, x " optimised");\
	} while (0)
#else
#define	DEBUG_FNCOUNT(x)	((void)x)
#endif


/* blend mask x color -> dst */

#ifdef BUILD_NEON
static void
_op_blend_mas_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   // main loop process data in pairs, so we need count to be even
   DATA32 *e = d + l - (l % 2);

   // everything we can do only once per cycle
   // loading of 'c', initialization of some registers
   __asm__ __volatile__
   (
       ".fpu neon                                      \n\t"
       "       vmov.32         d30[0], %[c]            \n\t"
       "       vmov.i16        q10,    #255            \n\t"
       "       vmov.i16        q11,    #256            \n\t"
       "       veor            d29,    d29, d29        \n\t"
       "       vzip.8          d30,    d29             \n\t"
       "       vmov            d31,    d30             \n\t"
     :
     : [c] "r" (c)
     : "q10", "q11", "q15", "d29"
   );
   while (d < e)
     {
        // main cycle
        __asm__ __volatile__
        (
            // load pair '*d' and '*(d+1)' into vector register
            "       vldm            %[d],   {d4}            \n\t"

            // load '*m' and '*(m+1)'
            "       veor            q0,     q0, q0          \n\t"
            "       vld1.8          d0[0],  [%[m]]!         \n\t"
            "       vld1.8          d1[0],  [%[m]]!         \n\t"

            // spread values from d in vector registers so for each
            // 8 bit channel data we have 8 bit of zeros
            // so each 32bit value occupies now one 64 bit register
            "       veor            d5,     d5, d5          \n\t"
            "       vzip.8          d4,     d5              \n\t"

            // copy *m values in corresponding registers
            "       vdup.u16        d0,     d0[0]           \n\t"
            "       vdup.u16        d1,     d1[0]           \n\t"

            // multiply a * c
            "       vmul.u16        q13,    q0, q15         \n\t"
            "       vadd.i16        q13,    q13, q10        \n\t"
            "       vsri.16         q13,    q13, #8         \n\t"
            "       vand            q13,    q13, q10        \n\t"

            // extract negated alpha
            "       vdup.u16        d24,    d26[3]          \n\t"
            "       vdup.u16        d25,    d27[3]          \n\t"
            "       vsub.i16        q12,    q11, q12        \n\t"

            // multiply alpha * (*d) and add a*c
            "       vmul.u16        q2,     q2, q12         \n\t"
            "       vsri.16         q2,     q2, #8          \n\t"
            "       vand            q2,     q2, q10         \n\t"
            "       vadd.i16        q2,     q2, q13         \n\t"
            "       vand            q2,     q2, q10         \n\t"

            // save results
            "       vqmovn.u16      d4,     q2              \n\t"
            "       vstm            %[d]!,  {d4}            \n\t"
          : [d] "+r" (d), [m] "+r" (m)
          : [c] "r" (c)
          : "q0", "q2", "q15", "q13", "q12", "q11", "q10",
            "memory"
        );
     }
   if (l % 2)
     {
        // do analogue of main loop for last element, if needed
        __asm__ __volatile__
        (
            "       vld1.32         d4[0],  [%[d]]          \n\t"

            "       veor            d0,     d0, d0          \n\t"
            "       vld1.8          d0[0],  [%[m]]!         \n\t"

            "       veor            d5,     d5, d5          \n\t"
            "       vzip.8          d4,     d5              \n\t"

            "       vdup.u16        d0,     d0[0]           \n\t"

            "       vmul.u16        d26,    d0, d30         \n\t"
            "       vadd.i16        d26,    d26, d20        \n\t"
            "       vsri.16         d26,    d26, #8         \n\t"
            "       vand            d26,    d26, d20        \n\t"

            "       vdup.u16        d24,    d26[3]          \n\t"

            "       vsub.i16        d24,    d22, d24        \n\t"
            "       vmul.u16        d4,     d4, d24         \n\t"
            "       vsri.16         d4,     d4, #8          \n\t"
            "       vand            d4,     d4, d20         \n\t"
            "       vadd.i16        d4,     d4, d26         \n\t"
            "       vand            d4,     d4, d20         \n\t"

            "       vqmovn.u16      d4,     q2              \n\t"
            "       vst1.32         {d4[0]}, [%[d]]!        \n\t"
          : [d] "+r" (d), [m] "+r" (m)
          : [c] "r" (c)
          : "q0", "q2", "q15", "q13", "q12", "q11", "q10",
            "memory"
        );
     }
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_mas_can_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e,*tmp;
   int alpha;

   DEBUG_FNCOUNT("");

#define AP	"_blend_mas_can_dp_neon_"
     asm volatile (
		".fpu neon				\n\t"
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
	"	beq		"AP"dualstart		\n\t"


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

	"	andS		%[tmp], %[d],	$0xf	\n\t"
	"	beq		"AP"quadloop		\n\t"

	AP"dualstart:					\n\t"
	"	sub		%[tmp], %[e], %[d]	\n\t"
	"	cmp		%[tmp],	#16		\n\t"
	"	blt		"AP"loopout		\n\t"

	AP"dualloop:					\n\t"
	"	vld1.16		d0[0],	[%[m]]!		\n\t"
	"	vldm		%[d],	{d8}		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmul.u32	d0,	d0,	d30	\n\t"
	"	vshr.u8		d0,	d0, #1		\n\t"
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
	"	blt		"AP"loopout		\n\t"

	AP"quadloopint:					\n\t"
		// Load the mask: 4 bytes: It has d0/d1
	"	ldr		%[x],	[%[m]]		\n\t"
	"	add		%[m], #4		\n\t"

		// Check for shortcuts
	"	cmp		%[x],	#0		\n\t"
	"	beq		"AP"fastloop		\n\t"

	"	cmp		%[x],	$0xffffffff	\n\t"
	"	beq		"AP"quadstore		\n\t"

	"	vmov.32		d0[0],	%[x]		\n\t"
		// Load d into d8/d9 q4
	"	vldm		%[d],	{d8,d9}		\n\t"

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

	"	vstm		%[d]!,	{d8,d9}		\n\t"

	"	cmp		%[tmp], %[d]		\n\t"
	"	bhi		"AP"quadloopint		\n\t"
	"	b		"AP"loopout		\n\t"

	AP"quadstore:					\n\t"
	"	vstm		%[d]!,	{d18,d19}	\n\t"
	"	cmp		%[tmp], %[d]		\n\t"
	"	bhi		"AP"quadloopint		\n\t"

	AP"loopout:					\n\t"
#if NEONDEBUG
		"cmp		%[d], %[e]		\n\t"
		"ble		"AP"foo		\n\t"
		"sub		%[tmp],	%[tmp]		\n\t"
		"vst1.32	d0[0], [%[tmp]]		\n\t"
	AP"foo: \n\t"
#endif

	"	cmp		%[e], %[d]		\n\t"
	"	beq		"AP"done		\n\t"

	"	sub		%[tmp],%[e], %[d]	\n\t"
	"	cmp		%[tmp],#8		\n\t"

	"	blt		"AP"onebyte		\n\t"

		// Load the mask: 2 bytes: It has d0
	"	vld1.16		d0[0],	[%[m]]!		\n\t"

		// Load d into d8/d9 q4
	"	vldm		%[d],		{d8}	\n\t"

		// Get the alpha channel ready (m)
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmovl.u8	q0,	d0		\n\t"
	"	vmul.u32	d0,	d0,	d30	\n\t"
		// Lop a bit off to prevent overflow
	"	vshr.u8	d0,	d0, #1			\n\t"

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
		"vld1.8		d0[0],	[%[m]]!		\n\t"
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
#if NEONDEBUG
		"cmp		%[d], %[e]		\n\t"
		"beq		"AP"reallydone		\n\t"
		"sub		%[m],	%[m]		\n\t"
		"vst1.32	d0[0], [%[m]]		\n\t"
	AP"reallydone:"
#endif


	  : // output regs
	  // Input
          :  [e] "r" (e = d + l), [d] "r" (d), [c] "r" (c),
		[m] "r" (m), [tmp] "r" (7), [x] "r" (33)
          : "q0", "q1", "q2","q3", "q4","q5","q6", "q7","q9","q14","q15",
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
_op_blend_rel_mas_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;

   DEBUG_FNCOUNT("not");

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
