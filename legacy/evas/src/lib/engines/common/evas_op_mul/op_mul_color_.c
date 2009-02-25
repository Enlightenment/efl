
/* mul color --> dst */

#ifdef BUILD_C
static void
_op_mul_c_dp(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++) {
      *d = MUL4_SYM(c, *d);
   }
}

static void
_op_mul_caa_dp(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   for (; d < e; d++) {
      *d = MUL_256(c, *d);
   }
}

#define _op_mul_can_dp _op_mul_c_dp

#define _op_mul_c_dpan _op_mul_c_dp
#define _op_mul_can_dpan _op_mul_can_dp
#define _op_mul_caa_dpan _op_mul_caa_dp

static void
init_mul_color_span_funcs_c(void)
{
   op_mul_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mul_c_dp;
   op_mul_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_mul_can_dp;
   op_mul_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mul_caa_dp;

   op_mul_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mul_c_dpan;
   op_mul_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_can_dpan;
   op_mul_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mul_pt_c_dp(DATA32 s __UNUSED__, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
	*d = MUL4_SYM(c, *d);
}

#define _op_mul_pt_can_dp _op_mul_pt_c_dp
#define _op_mul_pt_caa_dp _op_mul_pt_c_dp

#define _op_mul_pt_c_dpan _op_mul_pt_c_dp
#define _op_mul_pt_can_dpan _op_mul_pt_can_dp
#define _op_mul_pt_caa_dpan _op_mul_pt_caa_dp

static void
init_mul_color_pt_funcs_c(void)
{
   op_mul_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mul_pt_c_dp;
   op_mul_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_mul_pt_can_dp;
   op_mul_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mul_pt_caa_dp;

   op_mul_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mul_pt_c_dpan;
   op_mul_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_pt_can_dpan;
   op_mul_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_pt_caa_dpan;
}
#endif

