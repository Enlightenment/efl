
/* copy mask x color -> dst */

#ifdef BUILD_NEON
static void
_op_copy_mas_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   int color;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        color = *m;
                        switch(color)
                          {
                          case 0:
                             break;
                          case 255:
                             *d = c;
                             break;
                          default:
                             color++;
                             *d = INTERP_256(color, c, *d);
                             break;
                          }
                        m++;  d++;
                     });
}

#define _op_copy_mas_cn_dp_neon _op_copy_mas_c_dp_neon
#define _op_copy_mas_can_dp_neon _op_copy_mas_c_dp_neon
#define _op_copy_mas_caa_dp_neon _op_copy_mas_c_dp_neon

#define _op_copy_mas_c_dpan_neon _op_copy_mas_c_dp_neon
#define _op_copy_mas_cn_dpan_neon _op_copy_mas_c_dpan_neon
#define _op_copy_mas_can_dpan_neon _op_copy_mas_c_dpan_neon
#define _op_copy_mas_caa_dpan_neon _op_copy_mas_c_dpan_neon

static void
init_copy_mask_color_span_funcs_neon(void)
{
   op_copy_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_mas_cn_dp_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_copy_mas_c_dp_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_copy_mas_can_dp_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_copy_mas_caa_dp_neon;

   op_copy_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_mas_cn_dpan_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_mas_c_dpan_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_mas_can_dpan_neon;
   op_copy_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_mas_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_mas_c_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   *d = INTERP_256(m + 1, c, *d);
}

#define _op_copy_pt_mas_cn_dp_neon _op_copy_pt_mas_c_dp_neon
#define _op_copy_pt_mas_can_dp_neon _op_copy_pt_mas_c_dp_neon
#define _op_copy_pt_mas_caa_dp_neon _op_copy_pt_mas_c_dp_neon

#define _op_copy_pt_mas_c_dpan_neon _op_copy_pt_mas_c_dp_neon
#define _op_copy_pt_mas_cn_dpan_neon _op_copy_pt_mas_c_dpan_neon
#define _op_copy_pt_mas_can_dpan_neon _op_copy_pt_mas_c_dpan_neon
#define _op_copy_pt_mas_caa_dpan_neon _op_copy_pt_mas_c_dpan_neon

static void
init_copy_mask_color_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_pt_mas_cn_dp_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_copy_pt_mas_c_dp_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_copy_pt_mas_can_dp_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_copy_pt_mas_caa_dp_neon;

   op_copy_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_mas_cn_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_pt_mas_c_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_pt_mas_can_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_pt_mas_caa_dpan_neon;
}
#endif

/*-----*/

/* copy_rel mask x color -> dst */

#ifdef BUILD_NEON
static void
_op_copy_rel_mas_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   int color;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        color = *m;
                        switch(color)
                          {
                          case 0:
                             break;
                          case 255:
                             color = 1 + (*d >> 24);
                             *d = MUL_256(color, c);
                             break;
                          default:
                               {
                                  DATA32 da = 1 + (*d >> 24);
                                  da = MUL_256(da, c);
                                  color++;
                                  *d = INTERP_256(color, da, *d);
                               }
                             break;
                          }
                        m++;  d++;
                     });
}

#define _op_copy_rel_mas_cn_dp_neon _op_copy_rel_mas_c_dp_neon
#define _op_copy_rel_mas_can_dp_neon _op_copy_rel_mas_c_dp_neon
#define _op_copy_rel_mas_caa_dp_neon _op_copy_rel_mas_c_dp_neon

#define _op_copy_rel_mas_c_dpan_neon _op_copy_mas_c_dpan_neon
#define _op_copy_rel_mas_cn_dpan_neon _op_copy_mas_cn_dpan_neon
#define _op_copy_rel_mas_can_dpan_neon _op_copy_mas_can_dpan_neon
#define _op_copy_rel_mas_caa_dpan_neon _op_copy_mas_caa_dpan_neon

static void
init_copy_rel_mask_color_span_funcs_neon(void)
{
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_mas_cn_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_copy_rel_mas_c_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_copy_rel_mas_can_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_copy_rel_mas_caa_dp_neon;

   op_copy_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_mas_cn_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_rel_mas_c_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_mas_can_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_mas_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_rel_pt_mas_c_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   s = 1 + (*d >> 24);
   s = MUL_256(s, c);
   *d = INTERP_256(m + 1, s, *d);
}

#define _op_copy_rel_pt_mas_cn_dp_neon _op_copy_rel_pt_mas_c_dp_neon
#define _op_copy_rel_pt_mas_can_dp_neon _op_copy_rel_pt_mas_c_dp_neon
#define _op_copy_rel_pt_mas_caa_dp_neon _op_copy_rel_pt_mas_c_dp_neon

#define _op_copy_rel_pt_mas_c_dpan_neon _op_copy_pt_mas_c_dpan_neon
#define _op_copy_rel_pt_mas_cn_dpan_neon _op_copy_pt_mas_cn_dpan_neon
#define _op_copy_rel_pt_mas_can_dpan_neon _op_copy_pt_mas_can_dpan_neon
#define _op_copy_rel_pt_mas_caa_dpan_neon _op_copy_pt_mas_caa_dpan_neon

static void
init_copy_rel_mask_color_pt_funcs_neon(void)
{
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_mas_cn_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_NEON] = _op_copy_rel_pt_mas_c_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_NEON] = _op_copy_rel_pt_mas_can_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_NEON] = _op_copy_rel_pt_mas_caa_dp_neon;

   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_cn_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_c_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_can_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pt_mas_caa_dpan_neon;
}
#endif
