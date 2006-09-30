
/* mask pixel x color --> dst */

#ifdef BUILD_C
static void
_op_mask_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   l = 1 + (c >> 24);
   while (d < e) {
	c = 1 + ((l * (*s >> 24)) >> 8);
	*d = MUL_256(c, *d);
	s++;  d++;
     }
}

#define _op_mask_pas_c_dp _op_mask_p_c_dp
#define _op_mask_pan_c_dp _op_mask_p_c_dp
#define _op_mask_p_can_dp _op_mask_p_c_dp
#define _op_mask_pas_can_dp _op_mask_p_c_dp
#define _op_mask_p_caa_dp _op_mask_p_c_dp
#define _op_mask_pas_caa_dp _op_mask_p_c_dp
#define _op_mask_pan_caa_dp _op_mask_p_c_dp

#define _op_mask_p_c_dpan _op_mask_p_c_dp
#define _op_mask_pas_c_dpan _op_mask_p_c_dp
#define _op_mask_pan_c_dpan _op_mask_p_c_dp
#define _op_mask_p_can_dpan _op_mask_p_c_dp
#define _op_mask_pas_can_dpan _op_mask_p_c_dp
#define _op_mask_p_caa_dpan _op_mask_p_c_dp
#define _op_mask_pas_caa_dpan _op_mask_p_c_dp
#define _op_mask_pan_caa_dpan _op_mask_p_c_dp

static void
init_mask_pixel_color_span_funcs_c(void)
{
   op_mask_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_mask_p_c_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_mask_pas_c_dp;
   op_mask_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_mask_pan_c_dp;
   op_mask_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_mask_p_can_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_mask_pas_can_dp;
   op_mask_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_mask_p_caa_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_mask_pas_caa_dp;
   op_mask_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_mask_pan_caa_dp;

   op_mask_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_mask_p_c_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_mask_pas_c_dpan;
   op_mask_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_mask_pan_c_dpan;
   op_mask_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_p_can_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pas_can_dpan;
   op_mask_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_p_caa_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pas_caa_dpan;
   op_mask_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pan_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + ((((c >> 24) * (s >> 24)) + 255) >> 8);
	*d = MUL_256(c, *d);
}

#define _op_mask_pt_pas_c_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_pan_c_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_p_can_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_can_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_p_caa_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_caa_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_pan_caa_dp _op_mask_pt_p_c_dp

#define _op_mask_pt_p_c_dpan _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_c_dpan _op_mask_pt_p_c_dp
#define _op_mask_pt_pan_c_dpan _op_mask_pt_p_c_dp
#define _op_mask_pt_p_can_dpan _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_can_dpan _op_mask_pt_p_c_dp
#define _op_mask_pt_p_caa_dpan _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_caa_dpan _op_mask_pt_p_c_dp
#define _op_mask_pt_pan_caa_dpan _op_mask_pt_p_c_dp

static void
init_mask_pixel_color_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_mask_pt_p_c_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_mask_pt_pas_c_dp;
   op_mask_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_mask_pt_pan_c_dp;
   op_mask_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_mask_pt_p_can_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_mask_pt_pas_can_dp;
   op_mask_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_p_caa_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_pas_caa_dp;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_pan_caa_dp;

   op_mask_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_p_c_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_pas_c_dpan;
   op_mask_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_pan_c_dpan;
   op_mask_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pt_p_can_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pt_pas_can_dpan;
   op_mask_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_p_caa_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_pas_caa_dpan;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_pan_caa_dpan;
}
#endif
