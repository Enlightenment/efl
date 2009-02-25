
/* mul mask x color -> dst */

#ifdef BUILD_C
static void
_op_mul_mas_c_dp(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, nc = ~c;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL4_SYM(c, *d);
		break;
	    default:
		a = ~MUL_SYM(a, nc);
		*d = MUL4_SYM(a, *d);
		break;
	  }
	m++;  d++;
     }
}

#define _op_mul_mas_can_dp _op_mul_mas_c_dp
#define _op_mul_mas_caa_dp _op_mul_mas_c_dp

#define _op_mul_mas_c_dpan _op_mul_mas_c_dp
#define _op_mul_mas_can_dpan _op_mul_mas_can_dp
#define _op_mul_mas_caa_dpan _op_mul_mas_caa_dp

static void
init_mul_mask_color_span_funcs_c(void)
{
   op_mul_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mul_mas_c_dp;
   op_mul_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_mul_mas_can_dp;
   op_mul_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mul_mas_caa_dp;

   op_mul_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mul_mas_c_dpan;
   op_mul_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_mul_mas_can_dpan;
   op_mul_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mul_mas_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mul_pt_mas_c_dp(DATA32 s __UNUSED__, DATA8 m, DATA32 c, DATA32 *d) {
	c = ~c;
	c = ~MUL_SYM(m, c);
	*d = MUL4_SYM(c, *d);
}

#define _op_mul_pt_mas_can_dp _op_mul_pt_mas_c_dp
#define _op_mul_pt_mas_caa_dp _op_mul_pt_mas_c_dp

#define _op_mul_pt_mas_c_dpan _op_mul_pt_mas_c_dp
#define _op_mul_pt_mas_can_dpan _op_mul_pt_mas_can_dp
#define _op_mul_pt_mas_caa_dpan _op_mul_pt_mas_caa_dp

static void
init_mul_mask_color_pt_funcs_c(void)
{
   op_mul_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mul_pt_mas_c_dp;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_mul_pt_mas_can_dp;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mul_pt_mas_caa_dp;

   op_mul_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mul_pt_mas_c_dpan;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_mul_pt_mas_can_dpan;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mul_pt_mas_caa_dpan;
}
#endif
