
/* blend color --> dst */

#ifdef BUILD_NEON
static void
_op_blend_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e, a = 256 - (c >> 24);
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = c + MUL_256(a, *d);
                        d++;
                     });
}

#define _op_blend_caa_dp_neon _op_blend_c_dp_neon

#define _op_blend_c_dpan_neon _op_blend_c_dp_neon
#define _op_blend_caa_dpan_neon _op_blend_c_dpan_neon

static void
init_blend_color_span_funcs_neon(void)
{
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_c_dp_neon;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_caa_dp_neon;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_c_dpan_neon;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_c_dp_neon(DATA32 s __UNUSED__, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   s = 256 - (c >> 24);
   *d = c + MUL_256(s, *d);
}

#define _op_blend_pt_caa_dp_neon _op_blend_pt_c_dp_neon

#define _op_blend_pt_c_dpan_neon _op_blend_pt_c_dp_neon
#define _op_blend_pt_caa_dpan_neon _op_blend_pt_c_dpan_neon

static void
init_blend_color_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_c_dp_neon;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_caa_dp_neon;

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_c_dpan_neon;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_caa_dpan_neon;
}
#endif
/*-----*/

/* blend_rel color -> dst */

#ifdef BUILD_NEON
static void
_op_blend_rel_c_dp_neon(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha = 256 - (c >> 24);
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = MUL_SYM(*d >> 24, c) + MUL_256(alpha, *d);
                        d++;
                     });
}

#define _op_blend_rel_caa_dp_neon _op_blend_rel_c_dp_neon

#define _op_blend_rel_c_dpan_neon _op_blend_c_dpan_neon
#define _op_blend_rel_caa_dpan_neon _op_blend_caa_dpan_neon

static void
init_blend_rel_color_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_rel_c_dp_neon;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_rel_caa_dp_neon;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_c_dpan_neon;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_rel_pt_c_dp_neon(DATA32 s __UNUSED__, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   s = *d >> 24;
   *d = MUL_SYM(s, c) + MUL_256(256 - (c >> 24), *d);
}

#define _op_blend_rel_pt_caa_dp_neon _op_blend_rel_pt_c_dp_neon

#define _op_blend_rel_pt_c_dpan_neon _op_blend_pt_c_dpan_neon
#define _op_blend_rel_pt_caa_dpan_neon _op_blend_pt_caa_dpan_neon

static void
init_blend_rel_color_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_blend_rel_pt_c_dp_neon;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_rel_pt_caa_dp_neon;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_c_dpan_neon;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_caa_dpan_neon;
}
#endif
