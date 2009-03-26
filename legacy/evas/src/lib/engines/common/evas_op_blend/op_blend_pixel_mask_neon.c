
/* blend pixel x mask --> dst */

#ifdef BUILD_NEON
static void
_op_blend_pas_mas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        alpha = (*s >> 24);
                        switch(alpha)
                          {
                          case 0:
                             break;
                          case 255:
                             *d = *s;
                             break;
                          default:
                             c = MUL_SYM(alpha, *s);
                             alpha = 256 - (c >> 24);
                             *d = c + MUL_256(alpha, *d);
                             break;
                          }
                     });
}

static void
_op_blend_pan_mas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        alpha = *m;
                        switch(alpha)
                          {
                          case 0:
                             break;
                          case 255:
                             *d = *s;
                             break;
                          default:
                             alpha++;
                             *d = INTERP_256(alpha, *s, *d);
                             break;
                          }
                        m++;  s++;  d++;
                     });
}

#define _op_blend_p_mas_dp_neon _op_blend_pas_mas_dp_neon

#define _op_blend_p_mas_dpan_neon _op_blend_p_mas_dp_neon
#define _op_blend_pan_mas_dpan_neon _op_blend_pan_mas_dp_neon
#define _op_blend_pas_mas_dpan_neon _op_blend_pas_mas_dp_neon

static void
init_blend_pixel_mask_span_funcs_neon(void)
{
   op_blend_span_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_p_mas_dp_neon;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pas_mas_dp_neon;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pan_mas_dp_neon;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_p_mas_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pas_mas_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pan_mas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_p_mas_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   s = MUL_SYM(m, s);
   c = 256 - (s >> 24);
   *d = s + MUL_256(c, *d);
}

#define _op_blend_pt_pan_mas_dp_neon _op_blend_pt_p_mas_dp_neon
#define _op_blend_pt_pas_mas_dp_neon _op_blend_pt_p_mas_dp_neon

#define _op_blend_pt_p_mas_dpan_neon _op_blend_pt_p_mas_dp_neon
#define _op_blend_pt_pas_mas_dpan_neon _op_blend_pt_pas_mas_dp_neon
#define _op_blend_pt_pan_mas_dpan_neon _op_blend_pt_pan_mas_dp_neon

static void
init_blend_pixel_mask_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pt_p_mas_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pt_pas_mas_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_blend_pt_pan_mas_dp_neon;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_p_mas_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_pas_mas_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_pt_pan_mas_dpan_neon;
}
#endif

/*-----*/

/* blend_rel pixel x mask -> dst */

#ifdef BUILD_NEON

#define _op_blend_rel_p_mas_dpan_neon _op_blend_p_mas_dpan_neon
#define _op_blend_rel_pas_mas_dpan_neon _op_blend_pas_mas_dpan_neon
#define _op_blend_rel_pan_mas_dpan_neon _op_blend_pan_mas_dpan_neon

static void
init_blend_rel_pixel_mask_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_p_mas_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pas_mas_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pan_mas_dpan_neon;
}
#endif

#ifdef BUILD_NEON

#define _op_blend_rel_pt_p_mas_dpan_neon _op_blend_pt_p_mas_dpan_neon
#define _op_blend_rel_pt_pas_mas_dpan_neon _op_blend_pt_pas_mas_dpan_neon
#define _op_blend_rel_pt_pan_mas_dpan_neon _op_blend_pt_pan_mas_dpan_neon

static void
init_blend_rel_pixel_mask_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_mas_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_mas_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_mas_dpan_neon;
}
#endif
