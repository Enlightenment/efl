
/* copy color --> dst */

#ifdef BUILD_NEON
static void
_op_copy_c_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   uint32_t *e;
   uint32_t dalign = ((uint32_t)d) & 0xf; // get alignment
   // handle unaligned stores - stores not aligned to 16bytes may suck
   if (dalign > 0)
     {
        dalign = (16 - dalign) >> 2;
        if (l < dalign) dalign = l;
        l -= dalign;
        e = d + dalign;
        for (; d < e; d++) {
           *d = c; // OP
        }
        if (l <= 0) return;
     }
   e = d + l;
#ifdef NEON_INSTRINSICS_OK
   e -= 15;
   // expand the color in c to a 128 bit register as "cccc" i.e 4 pixels of c
   uint32x4_t col = vdupq_n_u32(c);
   // fill a run of 4x4 (16) pixels with the color
   for (; d < e; d += 16) {
      vst1q_u32(d+0, col); // OP
      vst1q_u32(d+4, col); // OP
      vst1q_u32(d+8, col); // OP
      vst1q_u32(d+12, col); // OP
   }
   e += 15;
#else
   if ((e - d) >= 16)
     {
        e -= 31;
        asm volatile (
                      "vdup.32 q8, %[c]\n\t"
                      "asmloop1:\n\t"
//                      "pld [%[d], #128]\n\t"
                      "cmp %[e], %[d]\n\t"
                      "vst1.32 {d16-d17}, [%[d],:128]!\n\t"
                      "vst1.32 {d16-d17}, [%[d],:128]!\n\t"
                      "vst1.32 {d16-d17}, [%[d],:128]!\n\t"
                      "vst1.32 {d16-d17}, [%[d],:128]!\n\t"
                      "bhi asmloop1\n\t"
                      : // output regs
                      : [c] "r" (c), [e] "r" (e), [d] "r" (d) // input
                      : "q8", "d16", "d17", "memory" // clobbered
                      );
        e += 31;
     }
#endif
   // fixup any leftover pixels in the run
   for (; d < e; d++) {
      *d = c; // OP
   }
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
