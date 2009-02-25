
/* blend mask x color -> dst */

#ifdef BUILD_C
static void
_op_blend_mas_c_dp(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   l = 256 - (c >> 24);
   while (d < e) {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = c + MUL_256(l, *d);
		break;
	    default:
	      {
		DATA32 mc = MUL_SYM(a, c);
		a = 256 - (mc >> 24);
		*d = mc + MUL_256(a, *d);
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_blend_mas_can_dp(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    case 255:
		*d = c;
		break;
	    default:
		l++;
		*d = INTERP_256(l, c, *d);
		break;
	  }
	m++;  d++;
     }
}

#define _op_blend_mas_cn_dp _op_blend_mas_can_dp
#define _op_blend_mas_caa_dp _op_blend_mas_c_dp

#define _op_blend_mas_c_dpan _op_blend_mas_c_dp
#define _op_blend_mas_cn_dpan _op_blend_mas_cn_dp
#define _op_blend_mas_can_dpan _op_blend_mas_can_dp
#define _op_blend_mas_caa_dpan _op_blend_mas_caa_dp

static void
init_blend_mask_color_span_funcs_c(void)
{
   op_blend_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_mas_c_dp;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_mas_cn_dp;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_mas_can_dp;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_mas_caa_dp;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_mas_c_dpan;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_mas_cn_dpan;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_mas_can_dpan;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_mas_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL_SYM(m, c);
	m = 256 - (s >> 24);
	*d = s + MUL_256(m, *d);
}

static void
_op_blend_pt_mas_can_dp(DATA32 s __UNUSED__, DATA8 m, DATA32 c, DATA32 *d) {
	*d = INTERP_256(m + 1, c, *d);
}

#define _op_blend_pt_mas_cn_dp _op_blend_pt_mas_can_dp
#define _op_blend_pt_mas_caa_dp _op_blend_pt_mas_c_dp

#define _op_blend_pt_mas_c_dpan _op_blend_pt_mas_c_dp
#define _op_blend_pt_mas_cn_dpan _op_blend_pt_mas_cn_dp
#define _op_blend_pt_mas_can_dpan _op_blend_pt_mas_can_dp
#define _op_blend_pt_mas_caa_dpan _op_blend_pt_mas_caa_dp

static void
init_blend_mask_color_pt_funcs_c(void)
{
   op_blend_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_pt_mas_c_dp;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_pt_mas_cn_dp;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_pt_mas_can_dp;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_pt_mas_caa_dp;

   op_blend_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_pt_mas_c_dpan;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_pt_mas_cn_dpan;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_pt_mas_can_dpan;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_pt_mas_caa_dpan;
}
#endif

/*-----*/

/* blend_rel mask x color --> dst */

#ifdef BUILD_C
static void
_op_blend_rel_mas_c_dp(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
	DATA32 mc = MUL_SYM(*m, c);
	l = 256 - (mc >> 24);
	*d = MUL_SYM(*d >> 24, mc) + MUL_256(l, *d);
	d++;
	m++;
     }
}

#define _op_blend_rel_mas_cn_dp _op_blend_rel_mas_c_dp
#define _op_blend_rel_mas_can_dp _op_blend_rel_mas_c_dp
#define _op_blend_rel_mas_caa_dp _op_blend_rel_mas_c_dp

#define _op_blend_rel_mas_c_dpan _op_blend_mas_c_dpan
#define _op_blend_rel_mas_cn_dpan _op_blend_mas_cn_dpan
#define _op_blend_rel_mas_can_dpan _op_blend_mas_can_dpan
#define _op_blend_rel_mas_caa_dpan _op_blend_mas_caa_dpan

static void
init_blend_rel_mask_color_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_rel_mas_c_dp;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_mas_can_dp;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_rel_mas_can_dp;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_rel_mas_caa_dp;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_rel_mas_c_dpan;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_mas_cn_dpan;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_rel_mas_can_dpan;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_rel_mas_caa_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_blend_rel_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL_SYM(m, c);
	c = 256 - (s >> 24);
	*d = MUL_SYM(*d >> 24, s) + MUL_256(c, *d);
}

#define _op_blend_rel_pt_mas_cn_dp _op_blend_rel_pt_mas_c_dp
#define _op_blend_rel_pt_mas_can_dp _op_blend_rel_pt_mas_c_dp
#define _op_blend_rel_pt_mas_caa_dp _op_blend_rel_pt_mas_c_dp

#define _op_blend_rel_pt_mas_c_dpan _op_blend_pt_mas_c_dpan
#define _op_blend_rel_pt_mas_cn_dpan _op_blend_pt_mas_cn_dpan
#define _op_blend_rel_pt_mas_can_dpan _op_blend_pt_mas_can_dpan
#define _op_blend_rel_pt_mas_caa_dpan _op_blend_pt_mas_caa_dpan

static void
init_blend_rel_mask_color_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_rel_pt_mas_c_dp;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_pt_mas_cn_dp;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_rel_pt_mas_can_dp;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_rel_pt_mas_caa_dp;

   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_rel_pt_mas_c_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_mas_cn_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_rel_pt_mas_can_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pt_mas_caa_dpan;
}
#endif
