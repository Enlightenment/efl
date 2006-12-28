
/* copy pixel x color --> dst */

#ifdef BUILD_C
static void
_op_copy_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
	*d = MUL4_SYM(c, *s);
	d++;
	s++;
     }
}

static void
_op_copy_p_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e) {
	*d = MUL_256(c, *s);
	d++;
	s++;
     }
}


#define _op_copy_pas_c_dp _op_copy_p_c_dp
#define _op_copy_pan_c_dp _op_copy_p_c_dp
#define _op_copy_p_can_dp _op_copy_p_c_dp
#define _op_copy_pas_can_dp _op_copy_p_can_dp
#define _op_copy_pan_can_dp _op_copy_p_c_dp
#define _op_copy_pas_caa_dp _op_copy_p_caa_dp
#define _op_copy_pan_caa_dp _op_copy_p_caa_dp

#define _op_copy_p_c_dpan _op_copy_p_c_dp
#define _op_copy_pas_c_dpan _op_copy_pas_c_dp
#define _op_copy_pan_c_dpan _op_copy_pan_c_dp
#define _op_copy_p_can_dpan _op_copy_p_can_dp
#define _op_copy_pas_can_dpan _op_copy_pas_can_dp
#define _op_copy_pan_can_dpan _op_copy_pan_can_dp
#define _op_copy_p_caa_dpan _op_copy_p_caa_dp
#define _op_copy_pas_caa_dpan _op_copy_pas_caa_dp
#define _op_copy_pan_caa_dpan _op_copy_pan_caa_dp

static void
init_copy_pixel_color_span_funcs_c(void)
{
   op_copy_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_p_c_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_pas_c_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_pan_c_dp;
   op_copy_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_p_can_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_pas_can_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_pan_can_dp;
   op_copy_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_p_caa_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_pas_caa_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_pan_caa_dp;

   op_copy_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_p_c_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_pas_c_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_pan_c_dpan;
   op_copy_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_p_can_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pas_can_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pan_can_dpan;
   op_copy_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_p_caa_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pas_caa_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pan_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_copy_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL4_SYM(c, s);
}

static void
_op_copy_pt_p_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_SYM(c >> 24, s);
}


#define _op_copy_pt_p_can_dp _op_copy_pt_p_c_dp
#define _op_copy_pt_pan_c_dp _op_copy_pt_p_c_dp
#define _op_copy_pt_pan_can_dp _op_copy_pt_p_c_dp
#define _op_copy_pt_pan_caa_dp _op_copy_pt_p_caa_dp
#define _op_copy_pt_pas_c_dp _op_copy_pt_p_c_dp
#define _op_copy_pt_pas_can_dp _op_copy_pt_p_can_dp
#define _op_copy_pt_pas_caa_dp _op_copy_pt_p_caa_dp

#define _op_copy_pt_p_c_dpan _op_copy_pt_p_c_dp
#define _op_copy_pt_pas_c_dpan _op_copy_pt_pas_c_dp
#define _op_copy_pt_pan_c_dpan _op_copy_pt_pan_c_dp
#define _op_copy_pt_p_can_dpan _op_copy_pt_p_can_dp
#define _op_copy_pt_pas_can_dpan _op_copy_pt_pas_can_dp
#define _op_copy_pt_pan_can_dpan _op_copy_pt_pan_can_dp
#define _op_copy_pt_p_caa_dpan _op_copy_pt_p_caa_dp
#define _op_copy_pt_pas_caa_dpan _op_copy_pt_pas_caa_dp
#define _op_copy_pt_pan_caa_dpan _op_copy_pt_pan_caa_dp

static void
init_copy_pixel_color_pt_funcs_c(void)
{
   op_copy_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_pt_p_c_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_pt_pas_c_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_pt_pan_c_dp;
   op_copy_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_pt_p_can_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_pt_pas_can_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_pt_pan_can_dp;
   op_copy_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_pt_p_caa_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_pt_pas_caa_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_pt_pan_caa_dp;

   op_copy_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_pt_p_c_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_pt_pas_c_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_pt_pan_c_dpan;
   op_copy_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pt_p_can_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pt_pas_can_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pt_pan_can_dpan;
   op_copy_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pt_p_caa_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pt_pas_caa_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pt_pan_caa_dpan;
}
#endif

/*-----*/

/* copy_rel pixel x color --> dst */

#ifdef BUILD_C
static void
_op_copy_rel_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
	DATA32 cs = MUL4_SYM(c, *s);
	*d = MUL_SYM(*d >> 24, cs);
	d++;
	s++;
     }
}

#define _op_copy_rel_pas_c_dp _op_copy_rel_p_c_dp
#define _op_copy_rel_pan_c_dp _op_copy_rel_p_c_dp
#define _op_copy_rel_p_can_dp _op_copy_rel_p_c_dp
#define _op_copy_rel_pas_can_dp _op_copy_rel_pas_c_dp
#define _op_copy_rel_pan_can_dp _op_copy_rel_p_c_dp
#define _op_copy_rel_p_caa_dp _op_copy_rel_p_c_dp
#define _op_copy_rel_pas_caa_dp _op_copy_rel_p_c_dp
#define _op_copy_rel_pan_caa_dp _op_copy_rel_p_c_dp

#define _op_copy_rel_p_c_dpan _op_copy_p_c_dpan
#define _op_copy_rel_pas_c_dpan _op_copy_pas_c_dpan
#define _op_copy_rel_pan_c_dpan _op_copy_pan_c_dpan
#define _op_copy_rel_p_can_dpan _op_copy_p_can_dpan
#define _op_copy_rel_pas_can_dpan _op_copy_pas_can_dpan
#define _op_copy_rel_pan_can_dpan _op_copy_pan_can_dpan
#define _op_copy_rel_p_caa_dpan _op_copy_p_caa_dpan
#define _op_copy_rel_pas_caa_dpan _op_copy_pas_caa_dpan
#define _op_copy_rel_pan_caa_dpan _op_copy_pan_caa_dpan

static void
init_copy_rel_pixel_color_span_funcs_c(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_rel_p_c_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_rel_pas_c_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_rel_pan_c_dp;
   op_copy_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_p_can_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pas_can_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pan_can_dp;
   op_copy_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_p_caa_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pas_caa_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pan_caa_dp;

   op_copy_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_p_c_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pas_c_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pan_c_dpan;
   op_copy_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_p_can_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pas_can_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pan_can_dpan;
   op_copy_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_p_caa_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pas_caa_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pan_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_copy_rel_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL4_SYM(c, s);
	*d = MUL_SYM(*d >> 24, s);
}

#define _op_copy_rel_pt_pas_c_dp _op_copy_rel_pt_p_c_dp
#define _op_copy_rel_pt_pan_c_dp _op_copy_rel_pt_p_c_dp
#define _op_copy_rel_pt_p_can_dp _op_copy_rel_pt_p_c_dp
#define _op_copy_rel_pt_pas_can_dp _op_copy_rel_pt_p_can_dp
#define _op_copy_rel_pt_pan_can_dp _op_copy_rel_pt_p_c_dp
#define _op_copy_rel_pt_p_caa_dp _op_copy_rel_pt_p_c_dp
#define _op_copy_rel_pt_pas_caa_dp _op_copy_rel_pt_p_caa_dp
#define _op_copy_rel_pt_pan_caa_dp _op_copy_rel_pt_p_caa_dp

#define _op_copy_rel_pt_p_c_dpan _op_copy_pt_p_c_dpan
#define _op_copy_rel_pt_pas_c_dpan _op_copy_pt_pas_c_dpan
#define _op_copy_rel_pt_pan_c_dpan _op_copy_pt_pan_c_dpan
#define _op_copy_rel_pt_p_can_dpan _op_copy_pt_p_can_dpan
#define _op_copy_rel_pt_pas_can_dpan _op_copy_pt_pas_can_dpan
#define _op_copy_rel_pt_pan_can_dpan _op_copy_pt_pan_can_dpan
#define _op_copy_rel_pt_p_caa_dpan _op_copy_pt_p_caa_dpan
#define _op_copy_rel_pt_pas_caa_dpan _op_copy_pt_pas_caa_dpan
#define _op_copy_rel_pt_pan_caa_dpan _op_copy_pt_pan_caa_dpan

static void
init_copy_rel_pixel_color_pt_funcs_c(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_rel_pt_p_c_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_rel_pt_pas_c_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_rel_pt_pan_c_dp;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pt_p_can_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pt_pas_can_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pt_pan_can_dp;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pt_p_caa_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pt_pas_caa_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pt_pan_caa_dp;

   op_copy_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pt_p_c_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pt_pas_c_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pt_pan_c_dpan;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_p_can_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_pas_can_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_pan_can_dpan;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_p_caa_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_pas_caa_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_pan_caa_dpan;
}
#endif
