
/* mask pixel --> dst */

#ifdef BUILD_C
static void
_op_mask_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
	*d = MUL_SYM(*s >> 24, *d);
   }
}

#define _op_mask_pas_dp _op_mask_p_dp

#define _op_mask_p_dpan _op_mask_p_dp
#define _op_mask_pas_dpan _op_mask_pas_dp

static void
init_mask_pixel_span_funcs_c(void)
{
   op_mask_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_mask_p_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_mask_pas_dp;

   op_mask_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_p_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pas_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_SYM(s >> 24, *d);
}

#define _op_mask_pt_pas_dp _op_mask_pt_p_dp

#define _op_mask_pt_p_dpan _op_mask_pt_p_dp
#define _op_mask_pt_pas_dpan _op_mask_pt_pas_dp

static void
init_mask_pixel_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_mask_pt_p_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_mask_pt_pas_dp;

   op_mask_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pt_p_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pt_pas_dpan;
}
#endif
