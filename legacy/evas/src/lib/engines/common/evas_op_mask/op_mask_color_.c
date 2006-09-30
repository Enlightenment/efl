

/* mask color --> dst */

#ifdef BUILD_C
static void
_op_mask_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   for (; d < e; d++) {
	*d = MUL_256(c, *d);
   }
}

#define _op_mask_caa_dp _op_mask_c_dp

#define _op_mask_c_dpan _op_mask_c_dp
#define _op_mask_caa_dpan _op_mask_caa_dp

static void
init_mask_color_span_funcs_c(void)
{
   op_mask_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mask_c_dp;
   op_mask_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mask_caa_dp;

   op_mask_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mask_c_dpan;
   op_mask_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_SYM(c >> 24, *d);
}

#define _op_mask_pt_caa_dp _op_mask_pt_c_dp

#define _op_mask_pt_c_dpan _op_mask_pt_c_dp
#define _op_mask_pt_caa_dpan _op_mask_pt_caa_dp

static void
init_mask_color_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mask_pt_c_dp;
   op_mask_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_caa_dp;

   op_mask_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_c_dpan;
   op_mask_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_caa_dpan;
}
#endif
