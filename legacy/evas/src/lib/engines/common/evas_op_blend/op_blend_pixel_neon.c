
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
   DATA32 *e = d + l;
   // this one... first
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
