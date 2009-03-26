
/* blend mask x color -> dst */

#ifdef BUILD_NEON
static void
_op_blend_mas_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha = 256 - (c >> 24);
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        DATA32 a = *m;
                        switch(a)
                          {
                          case 0:
                             break;
                          case 255:
                             *d = c + MUL_256(alpha, *d);
                             break;
                          default:
                               {
                                  DATA32 mc = MUL_SYM(a, c);
                                  a = 256 - (mc >> 24);
                                  *d = mc + MUL_256(a, *d);
                               }
                             break;
                          }
                        m++;  d++;
                     });
}

static void
_op_blend_mas_can_dp_neon(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
                             *d = c;
                             break;
                          default:
                             alpha++;
                             *d = INTERP_256(alpha, c, *d);
                             break;
                          }
                        m++;  d++;
                     });
}

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
