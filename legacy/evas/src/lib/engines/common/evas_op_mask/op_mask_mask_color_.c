
/* mask mask x color -> dst */

#ifdef BUILD_C
static void
_op_mask_mas_c_dp(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL_256(c, *d);
		break;
	    default:
		l = 256 - (((257 - c) * l) >> 8);
		*d = MUL_256(l, *d);
		break;
	  }
	m++;  d++;
     }
}

#define _op_mask_mas_caa_dp _op_mask_mas_c_dp

#define _op_mask_mas_c_dpan _op_mask_mas_c_dp
#define _op_mask_mas_caa_dpan _op_mask_mas_caa_dp

static void
init_mask_mask_color_span_funcs_c(void)
{
   op_mask_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mask_mas_c_dp;
   op_mask_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mask_mas_caa_dp;

   op_mask_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mask_mas_c_dpan;
   op_mask_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mask_mas_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_mas_c_dp(DATA32 s __UNUSED__, DATA8 m, DATA32 c, DATA32 *d) {
	c = 256 - (((256 - (c >> 24)) * m) >> 8);
	*d = MUL_256(c, *d);
}

#define _op_mask_pt_mas_caa_dp _op_mask_pt_mas_c_dp

#define _op_mask_pt_mas_c_dpan _op_mask_pt_mas_c_dp
#define _op_mask_pt_mas_caa_dpan _op_mask_pt_mas_caa_dp

static void
init_mask_mask_color_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mask_pt_mas_c_dp;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mask_pt_mas_caa_dp;

   op_mask_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mask_pt_mas_c_dpan;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mask_pt_mas_caa_dpan;
}
#endif
