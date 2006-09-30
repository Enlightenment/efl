
/* mul pixel x color --> dst */

#ifdef BUILD_C
static void
_op_mul_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
	DATA32 cs = MUL4_SYM(c, *s);
	*d = MUL4_SYM(cs, *d);
	s++;  d++;
     }
}

static void
_op_mul_p_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 cs = MUL_256(c, *s);
	*d = MUL4_SYM(cs, *d);
	s++;  d++;
     }
}

#define _op_mul_pas_c_dp _op_mul_p_c_dp
#define _op_mul_pan_c_dp _op_mul_p_c_dp
#define _op_mul_p_can_dp _op_mul_p_c_dp
#define _op_mul_pas_can_dp _op_mul_p_c_dp
#define _op_mul_pan_can_dp _op_mul_p_c_dp
#define _op_mul_pas_caa_dp _op_mul_p_caa_dp
#define _op_mul_pan_caa_dp _op_mul_p_caa_dp

#define _op_mul_p_c_dpan _op_mul_p_c_dp
#define _op_mul_pas_c_dpan _op_mul_pas_c_dp
#define _op_mul_pan_c_dpan _op_mul_pan_c_dp
#define _op_mul_p_can_dpan _op_mul_p_can_dp
#define _op_mul_pas_can_dpan _op_mul_pas_can_dp
#define _op_mul_pan_can_dpan _op_mul_pan_can_dp
#define _op_mul_p_caa_dpan _op_mul_p_caa_dp
#define _op_mul_pas_caa_dpan _op_mul_pas_caa_dp
#define _op_mul_pan_caa_dpan _op_mul_pan_caa_dp

static void
init_mul_pixel_color_span_funcs_c(void)
{
   op_mul_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_mul_p_c_dp;
   op_mul_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_mul_pas_c_dp;
   op_mul_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_mul_pan_c_dp;
   op_mul_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_mul_p_can_dp;
   op_mul_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_mul_pas_can_dp;
   op_mul_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_mul_pan_can_dp;
   op_mul_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_mul_p_caa_dp;
   op_mul_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_mul_pas_caa_dp;
   op_mul_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_mul_pan_caa_dp;

   op_mul_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_mul_p_c_dpan;
   op_mul_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_mul_pas_c_dpan;
   op_mul_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_mul_pan_c_dpan;
   op_mul_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_p_can_dpan;
   op_mul_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_pas_can_dpan;
   op_mul_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_pan_can_dpan;
   op_mul_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_p_caa_dpan;
   op_mul_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_pas_caa_dpan;
   op_mul_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_pan_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mul_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL4_SYM(c, s);
	*d = MUL4_SYM(s, *d);
}

static void
_op_mul_pt_p_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL_SYM(c >> 24, s);
	*d = MUL4_SYM(s, *d);
}

#define _op_mul_pt_pas_c_dp _op_mul_pt_p_c_dp
#define _op_mul_pt_pan_c_dp _op_mul_pt_p_c_dp
#define _op_mul_pt_p_can_dp _op_mul_pt_p_c_dp
#define _op_mul_pt_pas_can_dp _op_mul_pt_p_c_dp
#define _op_mul_pt_pan_can_dp _op_mul_pt_p_c_dp
#define _op_mul_pt_pas_caa_dp _op_mul_pt_p_caa_dp
#define _op_mul_pt_pan_caa_dp _op_mul_pt_p_caa_dp

#define _op_mul_pt_p_c_dpan _op_mul_pt_p_c_dp
#define _op_mul_pt_pan_c_dpan _op_mul_pt_pan_c_dp
#define _op_mul_pt_pas_c_dpan _op_mul_pt_pas_c_dp
#define _op_mul_pt_p_can_dpan _op_mul_pt_p_can_dp
#define _op_mul_pt_pan_can_dpan _op_mul_pt_pan_can_dp
#define _op_mul_pt_pas_can_dpan _op_mul_pt_pas_can_dp
#define _op_mul_pt_p_caa_dpan _op_mul_pt_p_caa_dp
#define _op_mul_pt_pan_caa_dpan _op_mul_pt_pan_caa_dp
#define _op_mul_pt_pas_caa_dpan _op_mul_pt_pas_caa_dp
static void
init_mul_pixel_color_pt_funcs_c(void)
{
   op_mul_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_mul_pt_p_c_dp;
   op_mul_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_mul_pt_pas_c_dp;
   op_mul_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_mul_pt_pan_c_dp;
   op_mul_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_mul_pt_p_can_dp;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_mul_pt_pas_can_dp;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_mul_pt_pan_can_dp;
   op_mul_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_mul_pt_p_caa_dp;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_mul_pt_pas_caa_dp;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_mul_pt_pan_caa_dp;

   op_mul_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_mul_pt_p_c_dpan;
   op_mul_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_mul_pt_pas_c_dpan;
   op_mul_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_mul_pt_pan_c_dpan;
   op_mul_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_pt_p_can_dpan;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_pt_pas_can_dpan;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_pt_pan_can_dpan;
   op_mul_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_pt_p_caa_dpan;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_pt_pas_caa_dpan;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_pt_pan_caa_dpan;
}
#endif
