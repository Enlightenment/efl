/* copy pixel --> dst */

#ifdef BUILD_NEON
static void
_op_copy_p_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e;
//   if (((unsigned long)s & 0xf) || ((unsigned long)d & 0xf))
//     {
        memcpy(d, s, l * sizeof(DATA32));
//        return;
//     }
/*
   e = d + l - 23;
   if (e > d)
     {
        int dl;
        asm volatile (
                      ".fpu neon                  \n\t"
                      "asmloop2:                  \n\t"
                      "cmp     %[e], %[d]         \n\t" // compare current and end ptr
                      "pld     [%[s], #64]        \n\t" // preload 64 bytes ahead
                      "pld     [%[s], #256]       \n\t" // preload 256 bytes ahead
                      "pld     [%[s], #320]       \n\t" // preload 320 bytes ahead
                      "vld1.64 {d0-d3},  [%[s]]!  \n\t" // load 256bits (32 bytes 8 pix)
                      "vld1.64 {d4-d7} , [%[s]]!  \n\t" // load 256bits (32 bytes 8 pix)
                      "vld1.64 {d8-d11}, [%[s]]!  \n\t" // load 256bits (32 bytes 8 pix)
                      "vst1.64 {d0-d3},  [%[d]]!  \n\t" // store 256bits (32 bytes 8 pix)
                      "vst1.64 {d4-d7},  [%[d]]!  \n\t" // store 256bits (32 bytes 8 pix)
                      "vst1.64 {d8-d11}, [%[d]]!  \n\t" // store 256bits (32 bytes 8 pix)
                      "bhi asmloop2               \n\t"
                      : // output regs
                      : [s] "r" (s), [e] "r" (e), [d] "r" (d) // input
                      : "q0", "q1", "q2", "q3", "q4", "q5", 
                      "d0", "d1", "d2", "d3", "d4", "d5", 
                      "d6", "d7", "d8", "d9", "d10", "d11",
                      "memory" // clobbered
                     );
        e = d + l;
        dl = l - (l % 24);
        s = s + dl;
        d = d + dl;
     }
   for (; d < e; d++, s++) {
      *d = *s;
   }
 */
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
