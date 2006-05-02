
/* blend color -> dst */

#ifdef BUILD_C
static void
_op_blend_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32  da = *d >> 24;
	da = 1 + _evas_pow_lut[((ca - 1) << 8) + da];
	*d++ = BLEND_ARGB_256(ca, da, c, *d);
     }
}

static void
_op_blend_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
	*d++ = BLEND_RGB_256(ca, c, *d);
}

#define _op_blend_caa_dp _op_blend_c_dp

#define _op_blend_caa_dpan _op_blend_c_dpan

#define _op_blend_c_dpas _op_blend_c_dp
#define _op_blend_caa_dpas _op_blend_c_dp

static void
init_blend_color_span_funcs_c(void)
{
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_blend_c_dp;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_blend_caa_dp;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_blend_c_dpan;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_caa_dpan;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_blend_c_dpas;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32  da = *d >> 24;
	da = 1 + _evas_pow_lut[((c >> 16) & 0xff00) + da];
	*d = BLEND_ARGB_256(1 + (c >> 24), da, c, *d);
}

static void
_op_blend_pt_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = BLEND_RGB_256(1 + (c >> 24), c, *d);
}

#define _op_blend_pt_caa_dp _op_blend_pt_c_dp

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

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_blend_pt_c_dpas;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_pt_caa_dpas;
}
#endif

/*-----*/

/* blend_rel color -> dst */

#ifdef BUILD_C
#define _op_blend_rel_c_dp _op_blend_c_dpan
#define _op_blend_rel_caa_dp _op_blend_caa_dpan

#define _op_blend_rel_c_dpan _op_blend_c_dpan
#define _op_blend_rel_caa_dpan _op_blend_caa_dpan

#define _op_blend_rel_c_dpas _op_blend_rel_c_dp
#define _op_blend_rel_caa_dpas _op_blend_rel_caa_dp

static void
init_blend_rel_color_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_blend_rel_c_dp;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_caa_dp;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_c_dpan;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_caa_dpan;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_c_dpas;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_caa_dpas;
}
#endif

#ifdef BUILD_C
#define _op_blend_rel_pt_c_dp _op_blend_pt_c_dpan
#define _op_blend_rel_pt_caa_dp _op_blend_pt_caa_dpan

#define _op_blend_rel_pt_c_dpan _op_blend_pt_c_dpan
#define _op_blend_rel_pt_caa_dpan _op_blend_pt_caa_dpan

#define _op_blend_rel_pt_c_dpas _op_blend_rel_pt_c_dp
#define _op_blend_rel_pt_caa_dpas _op_blend_rel_pt_caa_dp

static void
init_blend_rel_color_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_blend_rel_pt_c_dp;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_pt_caa_dp;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_pt_c_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pt_caa_dpan;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_pt_c_dpas;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_pt_caa_dpas;
}
#endif
