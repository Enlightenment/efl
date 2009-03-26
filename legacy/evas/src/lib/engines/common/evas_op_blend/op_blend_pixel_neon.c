
/* blend pixel --> dst */

#ifdef BUILD_NEON
static void
_op_blend_p_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
      l = 256 - (*s >> 24);
      *d = *s++ + MUL_256(l, *d);
      d++;
   }
}

static void
_op_blend_pas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
#if 0
#ifdef NEON_INSTRINSICS_OK
#else
   DATA32 *e = d + l;
   if (l >= 4)
     {
        e -= 4;
        asm volatile (
//                      "vmov.i32 q3, $0xff000000\n\t"
//                      "asmloop3:\n\t"
//                      "vld1.32 {d0-d1}, [%[s]]!\n\t"
//                      "vmov.32 q2, q0\n\t"
//                      "vand.32 q2, q2, q3\n\t"
//                      "vceq.i32 q2, q2, #0\n\t"
//                      "beq blank\n\t"
//                      "vmov.32 d3, d0\n\t"
//                      "vmovl.u8 q0, d1\n\t"
//                      "vmovl.u8 q1, d3\n\t"
//                      "\n\t"
//                      "vmovn.u16 d1, q0\n\t"
//                      "vmovn.u16 d3, q1\n\t"
//                      "vmov.32 d0, d3\n\t"
//                      "\n\t"
//                      "vst1.32 {d0-d1}, [%[d]]!\n\t"
                      
//                      "cmp %[e], %[d]\n\t" // if d < e ...
//                      "bhi asmloop3\n\t" // (if d < e) ... goto asmloop3
//                      "b done\n\t"
                      
//                      "blank:\n\t"
//                      "add %[s], %[s], #16\n\t"
//                      "add %[d], %[d], #16\n\t"
//                      "cmp %[e], %[d]\n\t" // if d < e ...
//                      "bhi asmloop3\n\t" // (if d < e) ... goto asmloop3
                      
//                      "done:\n\t"
                      "asmloop3:\n\t"
                      "vld4.8 {d0-d3}, [%[s]]\n\t" // d0-d3 = s
                      "vld4.8 {d4-d7}, [%[d]]\n\t" // d4-d7 = d
                      "vmvn.8  d31, d3\n\t" // d31 = 255 - s.a
                      "vmull.u8 q4, d31, d4\n\t"
                      "vmull.u8 q5, d31, d5\n\t"
                      "vmull.u8 q6, d31, d6\n\t"
                      "vmull.u8 q7, d31, d7\n\t"
                      "vrshr.u16 q8, q4, #8\n\t"
                      "vrshr.u16 q9, q5, #8\n\t"
                      "vraddhn.u16 d20, q4, q8\n\t"
                      "vrshr.u16 q8, q6, #8\n\t"
                      "vraddhn.u16 d21, q5, q9\n\t"
                      "vrshr.u16 q9, q7, #8\n\t"
                      "vraddhn.u16 d22, q6, q8\n\t"
                      "vraddhn.u16 d23, q7, q9\n\t"
                      "vqadd.u8 d20, d0, d20\n\t"
                      "vqadd.u8 d21, d1, d21\n\t"
                      "vqadd.u8 d22, d2, d22\n\t"
                      "vqadd.u8 d23, d3, d23\n\t"
                      "vst4.8 {d20-d23}, [%[d]]!\n\t"
                      "vst4.8 {d20-d23}, [%[d]]\n\t"
                      "add %[s], %[s], #4\n\t" // s++
                      "add %[d], %[d], #4\n\t" // d++
                      "cmp %[e], %[d]\n\t" // if d < e ...
                      "bhi asmloop3\n\t" // (if d < e) ... goto asmloop3
                      : // output regs
                      : [s] "r" (s), [e] "r" (e), [d] "r" (d) // input
                      : "d0", "d1", "memory" // clobbered
                      );
        e += 4;
     }
   while (d < e)
     {
        switch (*s & 0xff000000)
          {
          case 0:
             break;
          case 0xff000000:
             *d = *s;
             break;
          default :
             l = 256 - (*s >> 24);
             *d = *s + MUL_256(l, *d);
             break;
          }
        s++;  d++;
     }
#endif   
#else   
   DATA32 *e = d + l;
   while (d < e)
     {
        switch (*s & 0xff000000)
          {
          case 0:
             break;
          case 0xff000000:
             *d = *s;
             break;
          default :
             l = 256 - (*s >> 24);
             *d = *s + MUL_256(l, *d);
             break;
          }
        s++;  d++;
     }
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
