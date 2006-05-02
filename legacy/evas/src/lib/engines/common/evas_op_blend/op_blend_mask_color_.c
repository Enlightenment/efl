
/* blend mask x color -> dst */

#ifdef BUILD_C
static void
_op_blend_mas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		a = 1 + _evas_pow_lut[((ca - 1) << 8) | (*d >> 24)];
		*d = BLEND_ARGB_256(ca, a, c, *d);
		break;
	    default:
	      {
		DATA32 da;
		a = (a * ca) >> 8;
		da = 1 + _evas_pow_lut[(a << 8) | (*d >> 24)];
		a++;
		*d = BLEND_ARGB_256(a, da, c, *d);
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_blend_mas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = c;
		break;
	    default:
	      {
		DATA32 da = 1 + _evas_pow_lut[(a << 8) | (*d >> 24)];
		a++;
		*d = BLEND_ARGB_256(a, da, c, *d);
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_blend_mas_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = BLEND_RGB_256(ca, c, *d);
		break;
	    default:
		a = 1 + ((a * ca) >> 8);
		*d = BLEND_RGB_256(a, c, *d);
		break;
	  }
	m++;  d++;
     }
}

static void
_op_blend_mas_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = c;
		break;
	    default:
		a++;
		*d = BLEND_RGB_256(a, c, *d);
		break;
	  }
	m++;  d++;
     }
}

#define _op_blend_mas_cn_dp _op_blend_mas_can_dp
#define _op_blend_mas_caa_dp _op_blend_mas_c_dp

#define _op_blend_mas_cn_dpan _op_blend_mas_can_dpan
#define _op_blend_mas_caa_dpan _op_blend_mas_c_dpan

#define _op_blend_mas_c_dpas _op_blend_mas_c_dp
#define _op_blend_mas_cn_dpas _op_blend_mas_cn_dp
#define _op_blend_mas_can_dpas _op_blend_mas_can_dp
#define _op_blend_mas_caa_dpas _op_blend_mas_caa_dp

static void
init_blend_mask_color_span_funcs_c(void)
{
   op_blend_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_mas_c_dp;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_mas_can_dp;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_mas_can_dp;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_mas_c_dp;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_mas_c_dpan;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_mas_cn_dpan;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_mas_can_dpan;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_mas_caa_dpan;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_blend_mas_c_dpas;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_mas_cn_dpas;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_blend_mas_can_dpas;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_blend_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 da = ((m * (c >> 24)) + 255) & 0xff00;
	s = 1 + (da >> 8);
	da = 1 + _evas_pow_lut[da + (*d >> 24)];
	*d = BLEND_ARGB_256(s, da, c, *d);
}

static void
_op_blend_pt_mas_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 1 + _evas_pow_lut[(m << 8) + (*d >> 24)];
	*d = BLEND_ARGB_256(1 + m, s, c, *d);
}

static void
_op_blend_pt_mas_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 1 + (((m * (c >> 24)) + 255) >> 8);
	*d = BLEND_RGB_256(s, c, *d);
}

static void
_op_blend_pt_mas_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = BLEND_RGB_256(1 + m, c, *d);
}

#define _op_blend_pt_mas_cn_dp _op_blend_pt_mas_can_dp
#define _op_blend_pt_mas_caa_dp _op_blend_pt_mas_c_dp

#define _op_blend_pt_mas_cn_dpan _op_blend_pt_mas_can_dpan
#define _op_blend_pt_mas_caa_dpan _op_blend_pt_mas_c_dpan

#define _op_blend_pt_mas_c_dpas _op_blend_pt_mas_c_dp
#define _op_blend_pt_mas_cn_dpas _op_blend_pt_mas_cn_dp
#define _op_blend_pt_mas_can_dpas _op_blend_pt_mas_can_dp
#define _op_blend_pt_mas_caa_dpas _op_blend_pt_mas_caa_dp

static void
init_blend_mask_color_pt_funcs_c(void)
{
   op_blend_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_pt_mas_c_dp;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_pt_mas_can_dp;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_pt_mas_can_dp;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_pt_mas_c_dp;

   op_blend_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_pt_mas_c_dpan;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_pt_mas_cn_dpan;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_pt_mas_can_dpan;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_pt_mas_caa_dpan;

   op_blend_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_blend_pt_mas_c_dpas;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_pt_mas_cn_dpas;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_blend_pt_mas_can_dpas;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_blend_pt_mas_caa_dpas;
}
#endif

/*-----*/

/* blend_rel mask x color --> dst */

#ifdef BUILD_C
#define _op_blend_rel_mas_c_dp _op_blend_mas_c_dpan
#define _op_blend_rel_mas_cn_dp _op_blend_mas_cn_dpan
#define _op_blend_rel_mas_can_dp _op_blend_mas_can_dpan
#define _op_blend_rel_mas_caa_dp _op_blend_mas_caa_dpan

#define _op_blend_rel_mas_c_dpan _op_blend_mas_c_dpan
#define _op_blend_rel_mas_cn_dpan _op_blend_mas_cn_dpan
#define _op_blend_rel_mas_can_dpan _op_blend_mas_can_dpan
#define _op_blend_rel_mas_caa_dpan _op_blend_mas_caa_dpan

#define _op_blend_rel_mas_c_dpas _op_blend_rel_mas_c_dp
#define _op_blend_rel_mas_cn_dpas _op_blend_rel_mas_cn_dp
#define _op_blend_rel_mas_can_dpas _op_blend_rel_mas_can_dp
#define _op_blend_rel_mas_caa_dpas _op_blend_rel_mas_caa_dp

static void
init_blend_rel_mask_color_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_rel_mas_c_dp;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_mas_can_dp;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_rel_mas_can_dp;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_rel_mas_c_dp;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_rel_mas_c_dpan;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_mas_cn_dpan;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_rel_mas_can_dpan;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_rel_mas_caa_dpan;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_blend_rel_mas_c_dpas;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_mas_cn_dpas;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_blend_rel_mas_can_dpas;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_blend_rel_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
#define _op_blend_rel_pt_mas_c_dp _op_blend_pt_mas_c_dpan
#define _op_blend_rel_pt_mas_cn_dp _op_blend_pt_mas_cn_dpan
#define _op_blend_rel_pt_mas_can_dp _op_blend_pt_mas_can_dpan
#define _op_blend_rel_pt_mas_caa_dp _op_blend_pt_mas_caa_dpan

#define _op_blend_rel_pt_mas_c_dpan _op_blend_pt_mas_c_dpan
#define _op_blend_rel_pt_mas_cn_dpan _op_blend_pt_mas_cn_dpan
#define _op_blend_rel_pt_mas_can_dpan _op_blend_pt_mas_can_dpan
#define _op_blend_rel_pt_mas_caa_dpan _op_blend_pt_mas_caa_dpan

#define _op_blend_rel_pt_mas_c_dpas _op_blend_rel_pt_mas_c_dp
#define _op_blend_rel_pt_mas_cn_dpas _op_blend_rel_pt_mas_cn_dp
#define _op_blend_rel_pt_mas_can_dpas _op_blend_rel_pt_mas_can_dp
#define _op_blend_rel_pt_mas_caa_dpas _op_blend_rel_pt_mas_caa_dp

static void
init_blend_rel_mask_color_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_blend_rel_pt_mas_c_dp;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_pt_mas_can_dp;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_blend_rel_pt_mas_can_dp;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_blend_rel_pt_mas_c_dp;

   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_blend_rel_pt_mas_c_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_mas_cn_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_blend_rel_pt_mas_can_dpan;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pt_mas_caa_dpan;

   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_blend_rel_pt_mas_c_dpas;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_pt_mas_cn_dpas;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_blend_rel_pt_mas_can_dpas;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_blend_rel_pt_mas_caa_dpas;
}
#endif
