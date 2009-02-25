
/* blend color -> dst */

#ifdef BUILD_C
static void
_op_blend_c_dp(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, a = 256 - (c >> 24);
   while (d < e)
     {
	*d = c + MUL_256(a, *d);
	d++;
     }
}

#define _op_blend_caa_dp _op_blend_c_dp

#define _op_blend_c_dpan _op_blend_c_dp
#define _op_blend_caa_dpan _op_blend_c_dpan

static void
init_blend_color_span_funcs_c(void)
{
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_blend_c_dp;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_blend_caa_dp;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_blend_c_dpan;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_c_dp(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
	s = 256 - (c >> 24);
	*d = c + MUL_256(s, *d);
}

#define _op_blend_pt_caa_dp _op_blend_pt_c_dp

#define _op_blend_pt_c_dpan _op_blend_pt_c_dp
#define _op_blend_pt_caa_dpan _op_blend_pt_c_dpan

#define _op_blend_pt_c_dpas _op_blend_pt_c_dp
#define _op_blend_pt_caa_dpas _op_blend_pt_c_dp

static void
init_blend_color_pt_funcs_c(void)
{
   op_blend_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_blend_pt_c_dp;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_blend_pt_caa_dp;

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_blend_pt_c_dpan;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_pt_caa_dpan;
}
#endif

/*-----*/

/* blend_rel color -> dst */

#ifdef BUILD_C
static void
_op_blend_rel_c_dp(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   l = 256 - (c >> 24);
   while (d < e)
     {
	*d = MUL_SYM(*d >> 24, c) + MUL_256(l, *d);
	d++;
     }
}

#define _op_blend_rel_caa_dp _op_blend_rel_c_dp

#define _op_blend_rel_c_dpan _op_blend_c_dpan
#define _op_blend_rel_caa_dpan _op_blend_caa_dpan

static void
init_blend_rel_color_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_blend_rel_c_dp;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_caa_dp;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_c_dpan;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_blend_rel_pt_c_dp(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
	s = *d >> 24;
	*d = MUL_SYM(s, c) + MUL_256(256 - (c >> 24), *d);
}

#define _op_blend_rel_pt_caa_dp _op_blend_rel_pt_c_dp

#define _op_blend_rel_pt_c_dpan _op_blend_pt_c_dpan
#define _op_blend_rel_pt_caa_dpan _op_blend_pt_caa_dpan

static void
init_blend_rel_color_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_blend_rel_pt_c_dp;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_pt_caa_dp;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_pt_c_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pt_caa_dpan;
}
#endif
