#ifdef BUILD_NEON
#include <arm_neon.h>
#endif
#define NEONDEBUG 0


#if NEONDEBUG
#define DEBUG_FNCOUNT(x)	\
	do {			\
	static int _foo = 0;		\
	if (_foo++%10000 ==0)		\
		printf("%s %+d %s: %d (%s)\n",__FILE__,__LINE__,__func__,\
				_foo, x " optimised");\
	} while (0)
#else
#define	DEBUG_FNCOUNT(x)	((void)x)
#endif


/* blend mask x color -> dst */

#ifdef BUILD_NEON
static void
_op_blend_mas_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
#ifdef BUILD_NEON_INTRINSICS
   /* mc = MUL_SYM(m, c) per channel, i.e. (c * m + 255) >> 8, then
    * d = mc + MUL_256(256 - (mc >> 24), d).
    *
    * The second step is expressed as
    *
    *    mc + ((d * (255 - mc_a) + d) >> 8)
    *
    * because (256 - mc_a) does not fit in a byte, and d * (256 - mc_a) is the
    * same as d * (255 - mc_a) + d. Everything stays unsigned and inside 16
    * bits, so the whole span can be processed one channel plane at a time.
    *
    * Planar is what makes this cheap: vld4q gives one register per channel and
    * the per pixel mask lines up with each plane directly, so the packed
    * version's mask splat - two widenings plus a 32 bit multiply by 0x01010101
    * for every four pixels - is not needed at all.
    *
    * A mask of 0 needs no special case here, unlike the mask x can variant:
    * it gives mc = 0 and 255 - mc_a = 255, so the result is (d * 256) >> 8,
    * which is d. Fully transparent blocks are still skipped outright, since
    * that is free and glyph coverage masks are mostly zero. */
   const uint8x16_t c_b = vdupq_n_u8(c & 0xff);
   const uint8x16_t c_g = vdupq_n_u8((c >> 8) & 0xff);
   const uint8x16_t c_r = vdupq_n_u8((c >> 16) & 0xff);
   const uint8x16_t c_a = vdupq_n_u8((c >> 24) & 0xff);
   const uint16x8_t x255 = vdupq_n_u16(0xff);
   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~15);

   /* mc = (c * m + 255) >> 8 */
#define EVAS_MAS_C_MUL(out, cv)                                              \
   do {                                                                      \
      uint16x8_t lo_ = vmlal_u8(x255, vget_low_u8(cv), vget_low_u8(m8));      \
      uint16x8_t hi_ = vmlal_u8(x255, vget_high_u8(cv), vget_high_u8(m8));    \
      out = vcombine_u8(vshrn_n_u16(lo_, 8), vshrn_n_u16(hi_, 8));            \
   } while (0)

   /* d = mc + ((d * (255 - mc_a) + d) >> 8) */
#define EVAS_MAS_C_BLEND(plane, mc)                                          \
   do {                                                                      \
      uint16x8_t lo_ = vmull_u8(vget_low_u8(plane), vget_low_u8(nmca));       \
      uint16x8_t hi_ = vmull_u8(vget_high_u8(plane), vget_high_u8(nmca));     \
      lo_ = vaddw_u8(lo_, vget_low_u8(plane));                                \
      hi_ = vaddw_u8(hi_, vget_high_u8(plane));                               \
      plane = vaddq_u8(mc, vcombine_u8(vshrn_n_u16(lo_, 8),                   \
                                       vshrn_n_u16(hi_, 8)));                 \
   } while (0)

   while (start < end)
     {
        uint8x16_t m8 = vld1q_u8(m);
        uint8x16x4_t dp;
        uint8x16_t mc_b, mc_g, mc_r, mc_a, nmca;

        if (vmaxvq_u8(m8) == 0)          /* wholly transparent: nothing to do */
          {
             m += 16;
             start += 16;
             continue;
          }

        EVAS_MAS_C_MUL(mc_b, c_b);
        EVAS_MAS_C_MUL(mc_g, c_g);
        EVAS_MAS_C_MUL(mc_r, c_r);
        EVAS_MAS_C_MUL(mc_a, c_a);
        nmca = vmvnq_u8(mc_a);

        dp = vld4q_u8((const uint8_t *)start);
        EVAS_MAS_C_BLEND(dp.val[0], mc_b);
        EVAS_MAS_C_BLEND(dp.val[1], mc_g);
        EVAS_MAS_C_BLEND(dp.val[2], mc_r);
        EVAS_MAS_C_BLEND(dp.val[3], mc_a);
        vst4q_u8((uint8_t *)start, dp);

        m += 16;
        start += 16;
     }
#undef EVAS_MAS_C_MUL
#undef EVAS_MAS_C_BLEND

   end += (size & 15);
   while (start <  end) {
      DATA32 a = *m;
      DATA32 mc = MUL_SYM(a, c);
      a = 256 - (mc >> 24);
      *start = mc + MUL_256(a, *start);
      m++;  start++;
   }
#else
   DATA32 *e = d + l;

   // everything we can do only once per cycle
   // loading of 'c', initialization of some registers
   asm volatile (
        "       .fpu neon                                               \n\t"
        "       vdup.i32        q15,    %[c]                            \n\t"
        "       vmov.i8         q14,    #1                              \n\t"
        "       vmov.i16        q12,    #255                            \n\t"

                :
                : [c] "r" (c)
                : "q12", "q14", "q15"
   );
   //here we do unaligned part of 'd'
   while (((int)d & 0xf) && (d < e))
     {
        asm volatile (
                "       vld1.8          d0[0],  [%[m]]!                 \n\t"
                "       vld1.32         d4[0],  [%[d]]                  \n\t"
                "       vdup.u8         d0,     d0[0]                   \n\t"
                "       vmull.u8        q4,     d0, d30                 \n\t"
                "       vadd.u16        q4,     q4, q12                 \n\t"
                "       vshrn.u16       d12,    q4, #8                  \n\t"
                "       vmvn.u16        d14,    d12                     \n\t"
                "       vshr.u32        d16,    d14, #24                \n\t"
                "       vmul.u32        d16,    d16, d28                \n\t"
                "       vmovl.u8        q9,     d4                      \n\t"
                "       vmull.u8        q7,     d16, d4                 \n\t"
                "       vadd.u16        q7,     q9, q7                  \n\t"
                "       vshrn.u16       d0,     q7, #8                  \n\t"
                "       vadd.u8 d0,     d0,     d12                     \n\t"
                "       vst1.32         d0[0],  [%[d]]!                 \n\t"

                        : [d] "+r" (d), [m] "+r" (m)
                        : [c] "r" (c)
                        : "q0", "q2", "q4", "q5", "q6", "q7", "q8", "q9",
                                "q10", "q15", "q14", "memory"
        );
     }
   //here e - d should be divisible by 4
   while((unsigned int)d < ((unsigned int)e & 0xfffffff0))
     {
        //check if all 4 *m values are zeros
        int k = *((int *)m);
        if (k == 0)
          {
             m+=4;
             d+=4;
             continue;
          }

        asm volatile (
                        // load pair '*d' and '*(d+1)' into vector register
                "       vld1.32         d0[0],  [%[m]]!                 \n\t"
                "       vldm            %[d],   {q2}                    \n\t"
                "       vmovl.u8        q0,     d0                      \n\t"
                "       vmovl.u8        q0,     d0                      \n\t"
                "       vmul.u32        q0,     q14                     \n\t"

                        // Multiply     a * c
                "       vmull.u8        q4,     d0, d30                 \n\t"
                "       vadd.u16        q4,     q4, q12                 \n\t"
                "       vmull.u8        q5,     d1, d31                 \n\t"
                "       vadd.u16        q5,     q5, q12                 \n\t"

                        // Shorten
                "       vshrn.u16       d12,    q4, #8                  \n\t"
                "       vshrn.u16       d13,    q5, #8                  \n\t"

                        // extract negated alpha
                "       vmvn.u16        q7,     q6                      \n\t"
                "       vshr.u32        q8,     q7, #24                 \n\t"
                "       vmul.u32        q8,     q8, q14                 \n\t"

                        // Multiply
                "       vmovl.u8        q9,     d4                      \n\t"
                "       vmull.u8        q7,     d16, d4                 \n\t"
                "       vadd.u16        q7,     q9, q7                  \n\t"
                "       vmovl.u8        q10,    d5                      \n\t"
                "       vmull.u8        q8,     d17, d5                 \n\t"
                "       vadd.u16        q8,     q10, q8                 \n\t"

                "       vshrn.u16       d0,     q7, #8                  \n\t"
                "       vshrn.u16       d1,     q8, #8                  \n\t"

                        // Add
                "       vadd.u8 q0,     q0,     q6                      \n\t"

                "       vstm            %[d]!,  {d0,d1}                 \n\t"

                        : [d] "+r" (d), [m] "+r" (m)
                        : [c] "r" (c), [x] "r" (42)
                        : "q0", "q2", "q4", "q5", "q6", "q7", "q8", "q9",
                                "q10", "q15", "q14", "memory"
        );
     }
   //do the remaining part
   while (d < e)
     {
        asm volatile (
                "       vld1.8          d0[0],  [%[m]]!                 \n\t"
                "       vld1.32         d4[0],  [%[d]]                  \n\t"
                "       vdup.u8         d0,     d0[0]                   \n\t"
                "       vmull.u8        q4,     d0, d30                 \n\t"
                "       vadd.u16        q4,     q4, q12                 \n\t"
                "       vshrn.u16       d12,    q4, #8                  \n\t"
                "       vmvn.u16        d14,    d12                     \n\t"
                "       vshr.u32        d16,    d14, #24                \n\t"
                "       vmul.u32        d16,    d16, d28                \n\t"
                "       vmovl.u8        q9,     d4                      \n\t"
                "       vmull.u8        q7,     d16, d4                 \n\t"
                "       vadd.u16        q7,     q9, q7                  \n\t"
                "       vshrn.u16       d0,     q7, #8                  \n\t"
                "       vadd.u8 d0,     d0,     d12                     \n\t"
                "       vst1.32         d0[0],  [%[d]]!                 \n\t"

                        : [d] "+r" (d), [m] "+r" (m)
                        : [c] "r" (c)
                        : "q0", "q2", "q4", "q5", "q6", "q7", "q8", "q9",
                                "q10", "q15", "q14", "memory"
        );
    }
#endif
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_mas_can_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
#ifdef BUILD_NEON_INTRINSICS
   /* d = INTERP_256(m + 1, c, d) per channel, i.e.
    *
    *    d + (((c - d) * (m + 1)) >> 8)
    *
    * Rearranged so that no intermediate is ever negative:
    *
    *    (d * (255 - m) + c * (m + 1)) >> 8
    *
    * which is exact (the largest intermediate is 255*256 = 65280, so it stays
    * inside 16 bits) and lets the span be processed one channel plane at a
    * time. Planar is the point: vld4q hands back one register per channel and
    * the per pixel mask then lines up with each plane directly, so the byte
    * splat that the packed version needed - two widenings and a 32 bit
    * multiply by 0x01010101 for every four pixels - disappears entirely.
    *
    * A mask of 0 must leave the destination untouched rather than yield
    * (d * 255 + c) >> 8, so zero mask lanes are selected back to d. Whole
    * blocks that are entirely transparent or entirely opaque are handled
    * without any arithmetic at all, which is what the C path gets from its
    * switch and what glyph coverage masks mostly consist of. */
   const uint8x16_t c_b = vdupq_n_u8(c & 0xff);
   const uint8x16_t c_g = vdupq_n_u8((c >> 8) & 0xff);
   const uint8x16_t c_r = vdupq_n_u8((c >> 16) & 0xff);
   const uint8x16_t c_a = vdupq_n_u8((c >> 24) & 0xff);
   const uint32x4_t c_32x4 = vdupq_n_u32(c);
   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~15);

#define EVAS_MAS_CAN_CHAN(dst_plane, src_plane, cv)                          \
   do {                                                                      \
      uint16x8_t lo_ = vmull_u8(vget_low_u8(src_plane), vget_low_u8(nm));     \
      uint16x8_t hi_ = vmull_u8(vget_high_u8(src_plane), vget_high_u8(nm));   \
      lo_ = vmlal_u8(lo_, vget_low_u8(cv), vget_low_u8(m8));                  \
      hi_ = vmlal_u8(hi_, vget_high_u8(cv), vget_high_u8(m8));                \
      lo_ = vaddw_u8(lo_, vget_low_u8(cv));                                   \
      hi_ = vaddw_u8(hi_, vget_high_u8(cv));                                  \
      dst_plane = vbslq_u8(mzero,                                             \
                           src_plane,                                         \
                           vcombine_u8(vshrn_n_u16(lo_, 8),                   \
                                       vshrn_n_u16(hi_, 8)));                 \
   } while (0)

   while (start < end)
     {
        uint8x16_t m8 = vld1q_u8(m);
        uint8x16x4_t dp;
        uint8x16_t nm, mzero;

        if (vmaxvq_u8(m8) == 0)          /* wholly transparent: nothing to do */
          {
             m += 16;
             start += 16;
             continue;
          }
        if (vminvq_u8(m8) == 255)        /* wholly opaque: a flat colour fill */
          {
             vst1q_u32(start, c_32x4);
             vst1q_u32(start + 4, c_32x4);
             vst1q_u32(start + 8, c_32x4);
             vst1q_u32(start + 12, c_32x4);
             m += 16;
             start += 16;
             continue;
          }

        dp = vld4q_u8((const uint8_t *)start);
        nm = vmvnq_u8(m8);
        mzero = vceqq_u8(m8, vdupq_n_u8(0));

        EVAS_MAS_CAN_CHAN(dp.val[0], dp.val[0], c_b);
        EVAS_MAS_CAN_CHAN(dp.val[1], dp.val[1], c_g);
        EVAS_MAS_CAN_CHAN(dp.val[2], dp.val[2], c_r);
        EVAS_MAS_CAN_CHAN(dp.val[3], dp.val[3], c_a);

        vst4q_u8((uint8_t *)start, dp);
        m += 16;
        start += 16;
     }
#undef EVAS_MAS_CAN_CHAN

   end += (size & 15);
   while (start <  end) {
      DATA32 alpha = *m;
      switch (alpha)
        {
         case 0:
            break;
         case 255:
            *start = c;
            break;
         default:
            alpha++;
            *start = INTERP_256(alpha, c, *start);
            break;
        }
      m++;  start++;
   }
#else
   DATA32 *e;

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
#endif
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
   m = 255 - (s >> 24);
   *d = s + MUL_256(m, *d);
}

static void
_op_blend_pt_mas_can_dp_neon(DATA32 s EINA_UNUSED, DATA8 m, DATA32 c, DATA32 *d) {
   *d = INTERP_256(m + 1, c, *d);
}

#define _op_blend_pt_mas_cn_dp_neon _op_blend_pt_mas_can_dp_neon
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
#ifdef BUILD_NEON_INTRINSICS
/* The AArch64 path below uses tbl and the across-vector reduce, neither of
 * which exists in 32 bit NEON. Keep the original kernel for ARMv7 rather
 * than adding a second version that cannot be tested here. */
static void
_op_blend_rel_mas_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   /* mc = MUL_SYM(m, c), then
    *
    *    d = MUL_SYM(d_a, mc) + MUL_256(256 - mc_a, d)
    *
    * with the second term written as (d * (255 - mc_a) + d) >> 8 so that
    * 256 - mc_a never has to fit in a byte.
    *
    * Packed rather than planar, for the same reason as the pixel x mask
    * kernel: the C reference's final step adds two packed ARGB words, so an
    * overflowing channel carries into the next one, which a per byte add
    * would not reproduce.
    *
    * Both byte splats this needs - the mask byte across its pixel's channels,
    * and the destination alpha across its own - were two widenings plus a
    * 32 bit multiply by 0x01010101 per four pixels. Each is one tbl against a
    * constant index vector.
    *
    * A zero mask needs no special case: it gives mc = 0, so the first term is
    * 0 and the second is (d * 256) >> 8, which is d. Wholly transparent
    * blocks are skipped anyway because it is free. */
   static const uint8_t splat_idx[4][16] = {
      {  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3 },
      {  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7 },
      {  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11 },
      { 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15 }
   };
   static const uint8_t alpha_idx[16] =
      { 3, 3, 3, 3, 7, 7, 7, 7, 11, 11, 11, 11, 15, 15, 15, 15 };
   const uint8x16_t aidx = vld1q_u8(alpha_idx);
   const uint8x16_t c8 = vreinterpretq_u8_u32(vdupq_n_u32(c));
   const uint16x8_t x255 = vdupq_n_u16(0xff);
   DATA32 *start = d;
   int size = l;
   DATA32 *end = start + (size & ~15);

   while (start < end)
     {
        uint8x16_t m16 = vld1q_u8(m);
        int j;

        if (vmaxvq_u8(m16) == 0)
          {
             m += 16;
             start += 16;
             continue;
          }

        for (j = 0; j < 4; j++)
          {
             uint8x16_t d8 = vreinterpretq_u8_u32(vld1q_u32(start + (j * 4)));
             uint8x16_t m8 = vqtbl1q_u8(m16, vld1q_u8(splat_idx[j]));
             uint8x16_t mc, da, t, nmca, term;
             uint16x8_t lo, hi;

             /* mc = (c * m + 255) >> 8 */
             lo = vmlal_u8(x255, vget_low_u8(c8), vget_low_u8(m8));
             hi = vmlal_u8(x255, vget_high_u8(c8), vget_high_u8(m8));
             mc = vcombine_u8(vshrn_n_u16(lo, 8), vshrn_n_u16(hi, 8));

             /* t = (mc * d_a + 255) >> 8 */
             da = vqtbl1q_u8(d8, aidx);
             lo = vmlal_u8(x255, vget_low_u8(mc), vget_low_u8(da));
             hi = vmlal_u8(x255, vget_high_u8(mc), vget_high_u8(da));
             t = vcombine_u8(vshrn_n_u16(lo, 8), vshrn_n_u16(hi, 8));

             /* term = (d * (255 - mc_a) + d) >> 8 */
             nmca = vmvnq_u8(vqtbl1q_u8(mc, aidx));
             lo = vmull_u8(vget_low_u8(d8), vget_low_u8(nmca));
             hi = vmull_u8(vget_high_u8(d8), vget_high_u8(nmca));
             lo = vaddw_u8(lo, vget_low_u8(d8));
             hi = vaddw_u8(hi, vget_high_u8(d8));
             term = vcombine_u8(vshrn_n_u16(lo, 8), vshrn_n_u16(hi, 8));

             vst1q_u32(start + (j * 4),
                       vaddq_u32(vreinterpretq_u32_u8(t),
                                 vreinterpretq_u32_u8(term)));
          }

        m += 16;
        start += 16;
     }

   end += (size & 15);
   while (start < end)
   {
      DATA32 mc = MUL_SYM(*m, c);
      int alpha = 256 - (mc >> 24);
      *start = MUL_SYM(*start >> 24, mc) + MUL_256(alpha, *start);
      start++;
      m++;
   }
}
#else
static void
_op_blend_rel_mas_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   uint16x8_t dc0_16x8;
   uint16x8_t dc1_16x8;
   uint16x8_t m_16x8;
   uint16x8_t mc0_16x8;
   uint16x8_t mc1_16x8;
   uint16x8_t temp0_16x8;
   uint16x8_t temp1_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t c_32x2;
   uint32x2_t m_32x2 = { 0, 0 };
   uint32x4_t a_32x4;
   uint32x4_t ad_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t d_32x4;
   uint32x4_t dc_32x4;
   uint32x4_t m_32x4;
   uint32x4_t temp_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t a_8x16;
   uint8x16_t d_8x16;
   uint8x16_t dc_8x16;
   uint8x16_t m_8x16;
   uint8x16_t mc_8x16;
   uint8x16_t temp_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t a0_8x8;
   uint8x8_t a1_8x8;
   uint8x8_t c_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t dc0_8x8;
   uint8x8_t dc1_8x8;
   uint8x8_t m0_8x8;
   uint8x8_t m1_8x8;
   uint8x8_t m_8x8;
   uint8x8_t mc0_8x8;
   uint8x8_t mc1_8x8;
   uint8x8_t temp0_8x8;
   uint8x8_t temp1_8x8;

   c_32x2 = vdup_n_u32(c);
   c_8x8 = vreinterpret_u8_u32(c_32x2);
   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   x255_16x8 = vdupq_n_u16(0xff);
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

      // load 4 elements from m
      m_32x2 = vld1_lane_u32((DATA32*)m, m_32x2, 0);
      m_8x8 = vreinterpret_u8_u32(m_32x2);
      m_16x8 = vmovl_u8(m_8x8);
      m_8x16 = vreinterpretq_u8_u16(m_16x8);
      m_8x8 = vget_low_u8(m_8x16);
      m_16x8 = vmovl_u8(m_8x8);
      m_32x4 = vreinterpretq_u32_u16(m_16x8);

      m_32x4 = vmulq_u32(m_32x4, x1_32x4);
      m_8x16 = vreinterpretq_u8_u32(m_32x4);
      m0_8x8 = vget_low_u8(m_8x16);
      m1_8x8 = vget_high_u8(m_8x16);

      // multiply MUL_SYM(*m, c)
      mc0_16x8 = vmull_u8(m0_8x8, c_8x8);
      mc1_16x8 = vmull_u8(m1_8x8, c_8x8);
      mc0_16x8 = vaddq_u16(mc0_16x8, x255_16x8);
      mc1_16x8 = vaddq_u16(mc1_16x8, x255_16x8);
      mc0_8x8 = vshrn_n_u16(mc0_16x8, 8);
      mc1_8x8 = vshrn_n_u16(mc1_16x8, 8);
      mc_8x16 = vcombine_u8(mc0_8x8, mc1_8x8);

      // calculate alpha = 256 - (mc >> 24)
      a_8x16 = vsubq_u8(x0_8x16, mc_8x16);
      a_32x4 = vreinterpretq_u32_u8(a_8x16);
      a_32x4 = vshrq_n_u32(a_32x4, 24);
      a_32x4 = vmulq_u32(a_32x4, x1_32x4);
      a_8x16 = vreinterpretq_u8_u32(a_32x4);
      a0_8x8 = vget_low_u8(a_8x16);
      a1_8x8 = vget_high_u8(a_8x16);

      // multiply MUL_256(alpha, *d)
      temp0_16x8 = vmull_u8(a0_8x8, d0_8x8);
      temp1_16x8 = vmull_u8(a1_8x8, d1_8x8);
      temp0_8x8 = vshrn_n_u16(temp0_16x8,8);
      temp1_8x8 = vshrn_n_u16(temp1_16x8,8);
      temp_8x16 = vcombine_u8(temp0_8x8, temp1_8x8);
      temp_32x4 = vreinterpretq_u32_u8(temp_8x16);

      // select d where alpha == 0
      cond_32x4 = vceqq_u32(a_32x4, x0_32x4);
      ad_32x4 = vbslq_u32(cond_32x4, d_32x4, temp_32x4);

      // shift (*d >> 24)
      dc_32x4 = vshrq_n_u32(d_32x4, 24);
      dc_32x4 = vmulq_u32(x1_32x4, dc_32x4);
      dc_8x16 = vreinterpretq_u8_u32(dc_32x4);
      dc0_8x8 = vget_low_u8(dc_8x16);
      dc1_8x8 = vget_high_u8(dc_8x16);

      // multiply MUL_256(*d >> 24, sc);
      dc0_16x8 = vmull_u8(dc0_8x8, mc0_8x8);
      dc1_16x8 = vmull_u8(dc1_8x8, mc1_8x8);
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
      m+=4;
   }

   end += (l & 3);
   while (d < end)
   {
      DATA32 mc = MUL_SYM(*m, c);
      int alpha = 256 - (mc >> 24);
      *d = MUL_SYM(*d >> 24, mc) + MUL_256(alpha, *d);
      d++;
      m++;
   }
}
#endif

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
