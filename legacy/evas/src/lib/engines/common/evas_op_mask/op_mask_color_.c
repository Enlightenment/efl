

/* mask color --> dst */

#ifdef BUILD_C
static void
_op_mask_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l)
{
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   for (; d < e; d++) {
	*d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
   }
}

static void
_op_mask_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l)
{
   DATA32 *e = d + l;
   c |= 0x00ffffff;
   for (; d < e; d++) {
	*d &= c;
   }
}

#define _op_mask_cn_dp NULL
#define _op_mask_can_dp _op_mask_cn_dp
#define _op_mask_caa_dp _op_mask_c_dp

#define _op_mask_cn_dpan _op_mask_cn_dp
#define _op_mask_can_dpan _op_mask_can_dp
#define _op_mask_caa_dpan _op_mask_c_dpan

#define _op_mask_c_dpas _op_mask_c_dp
#define _op_mask_cn_dpas _op_mask_cn_dp
#define _op_mask_can_dpas _op_mask_can_dp
#define _op_mask_caa_dpas _op_mask_caa_dp

static void
init_mask_color_span_funcs_c(void)
{
   op_mask_span_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_mask_cn_dp;
   op_mask_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mask_c_dp;
   op_mask_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_mask_can_dp;
   op_mask_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mask_caa_dp;

   op_mask_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_cn_dpan;
   op_mask_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mask_c_dpan;
   op_mask_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_can_dpan;
   op_mask_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_caa_dpan;

   op_mask_span_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_cn_dpas;
   op_mask_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_mask_c_dpas;
   op_mask_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_can_dpas;
   op_mask_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(c >> 24, *d) + (*d & 0x00ffffff);
}

static void
_op_mask_pt_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d &= (c | 0x00ffffff);
}

#define _op_mask_pt_cn_dp NULL
#define _op_mask_pt_can_dp _op_mask_pt_cn_dp
#define _op_mask_pt_caa_dp _op_mask_pt_c_dp

#define _op_mask_pt_cn_dpan _op_mask_pt_cn_dp
#define _op_mask_pt_can_dpan _op_mask_pt_can_dp
#define _op_mask_pt_caa_dpan _op_mask_pt_c_dpan

#define _op_mask_pt_c_dpas _op_mask_pt_c_dp
#define _op_mask_pt_cn_dpas _op_mask_pt_cn_dp
#define _op_mask_pt_can_dpas _op_mask_pt_can_dp
#define _op_mask_pt_caa_dpas _op_mask_pt_caa_dp

static void
init_mask_color_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_mask_pt_cn_dp;
   op_mask_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mask_pt_c_dp;
   op_mask_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_mask_pt_can_dp;
   op_mask_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_caa_dp;

   op_mask_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pt_cn_dpan;
   op_mask_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_c_dpan;
   op_mask_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pt_can_dpan;
   op_mask_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_caa_dpan;

   op_mask_pt_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_pt_cn_dpas;
   op_mask_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_mask_pt_c_dpas;
   op_mask_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_pt_can_dpas;
   op_mask_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_pt_caa_dpas;
}
#endif
