
/* copy pixel --> dst */

#ifdef BUILD_NEON
static void
_op_copy_p_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   uint32_t *e;
   e = d + l;
//#ifdef NEON_INSTRINSICS_OK
#if 1
   // odd this is faster than the below asm... :(
   e -= 15;
   uint32x4_t col1, col2, col3, col4; 
   // fill a run of 4x4 (16) pixels with the color
   for (; d < e; d += 16, s += 16) {
      col1 = vld1q_u32(s+0); // OP
      col2 = vld1q_u32(s+4); // OP
      col3 = vld1q_u32(s+8); // OP
      col4 = vld1q_u32(s+12); // OP
      vst1q_u32(d+0, col1); // OP
      vst1q_u32(d+4, col2); // OP
      vst1q_u32(d+8, col3); // OP
      vst1q_u32(d+12, col4); // OP
   }
   e += 15;
#else
   if ((e - d) >= 16)
     {
        DATA32 *d2, *d3, *d4;
        DATA32 *s2, *s3, *s4;
        e -= 31;
        d2 = d + 4;
        d3 = d + 8;
        d4 = d + 12;
        s2 = s + 4;
        s3 = s + 8;
        s4 = s + 12;
        asm volatile (
                      "asmloop2:\n\t"
                      "cmp %[e], %[d]\n\t"
                      "vld1.32 {d16-d17}, [%[s]]!\n\t"
                      "vld1.32 {d18-d19}, [%[s2]]!\n\t"
                      "vld1.32 {d20-d21}, [%[s3]]!\n\t"
                      "vld1.32 {d22-d23}, [%[s4]]!\n\t"
                      "vst1.32 {d16-d17}, [%[d]]!\n\t"
                      "vst1.32 {d18-d19}, [%[d2]]!\n\t"
                      "vst1.32 {d20-d21}, [%[d3]]!\n\t"
                      "vst1.32 {d22-d23}, [%[d4]]!\n\t"
                      "bhi asmloop2\n\t"
                      : // output regs
                      : [s] "r" (s), [s2] "r" (s2), [s3] "r" (s3), [s4] "r" (s4), [e] "r" (e), [d] "r" (d), [d2] "r" (d2), [d3] "r" (d3), [d4] "r" (d4) // input
                      : "q8", "q9", "q10", "q11", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "memory" // clobbered
                      );
                e += 31;
     }
#endif   
   // fixup any leftover pixels in the run
   for (; d < e; d++, s++) {
      *d = *s; // OP
   }
}

#define _op_copy_pan_dp_neon _op_copy_p_dp_neon
#define _op_copy_pas_dp_neon _op_copy_p_dp_neon

#define _op_copy_p_dpan_neon _op_copy_p_dp_neon
#define _op_copy_pan_dpan_neon _op_copy_pan_dp_neon
#define _op_copy_pas_dpan_neon _op_copy_pas_dp_neon

static void
init_copy_pixel_span_funcs_neon(void)
{
   op_copy_span_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_p_dp_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pan_dp_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pas_dp_neon;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_p_dpan_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pan_dpan_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_p_dp_neon(DATA32 s, DATA8 m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d) {
   *d = s;
}

#define _op_copy_pt_pan_dp_neon _op_copy_pt_p_dp_neon
#define _op_copy_pt_pas_dp_neon _op_copy_pt_p_dp_neon

#define _op_copy_pt_p_dpan_neon _op_copy_pt_p_dp_neon
#define _op_copy_pt_pan_dpan_neon _op_copy_pt_pan_dp_neon
#define _op_copy_pt_pas_dpan_neon _op_copy_pt_pas_dp_neon

static void
init_copy_pixel_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_p_dp_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_pan_dp_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_pas_dp_neon;

   op_copy_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_p_dpan_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_pan_dpan_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_pas_dpan_neon;
}
#endif

/*-----*/

/* copy_rel pixel --> dst */

#ifdef BUILD_NEON
static void
_op_copy_rel_p_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = MUL_SYM(*d >> 24, c);
                        d++;
                     });
}


#define _op_copy_rel_pas_dp_neon _op_copy_rel_p_dp_neon
#define _op_copy_rel_pan_dp_neon _op_copy_rel_p_dp_neon

#define _op_copy_rel_p_dpan_neon _op_copy_p_dpan_neon
#define _op_copy_rel_pan_dpan_neon _op_copy_pan_dpan_neon
#define _op_copy_rel_pas_dpan_neon _op_copy_pas_dpan_neon

static void
init_copy_rel_pixel_span_funcs_neon(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_p_dp_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pan_dp_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pas_dp_neon;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_p_dpan_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pan_dpan_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_rel_pt_p_dp_neon(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   s = 1 + (*d >> 24);
   *d = MUL_256(s, c);
}


#define _op_copy_rel_pt_pan_dp_neon _op_copy_rel_pt_p_dp_neon
#define _op_copy_rel_pt_pas_dp_neon _op_copy_rel_pt_p_dp_neon

#define _op_copy_rel_pt_p_dpan_neon _op_copy_pt_p_dpan_neon
#define _op_copy_rel_pt_pan_dpan_neon _op_copy_pt_pan_dpan_neon
#define _op_copy_rel_pt_pas_dpan_neon _op_copy_pt_pas_dpan_neon

static void
init_copy_rel_pixel_pt_funcs_neon(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_p_dp_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_pan_dp_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_pas_dp_neon;

   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_p_dpan_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_pan_dpan_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_pas_dpan_neon;
}
#endif
