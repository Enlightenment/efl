
/* copy mask x color -> dst */

#ifdef BUILD_C
static void
_op_copy_mas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
		DATA32 aa = 1 + _evas_pow_lut[(a << 8) + (*d >> 24)];
		a++;
		*d = BLEND_ARGB_256(a, aa, c, *d);
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_copy_mas_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
		*d = INTERP_256(a, c, *d);
		break;
	  }
	m++;  d++;
     }
}


#define _op_copy_mas_cn_dp _op_copy_mas_c_dp
#define _op_copy_mas_can_dp _op_copy_mas_c_dp
#define _op_copy_mas_caa_dp _op_copy_mas_c_dp

#define _op_copy_mas_cn_dpan _op_copy_mas_c_dpan
#define _op_copy_mas_can_dpan _op_copy_mas_c_dpan
#define _op_copy_mas_caa_dpan _op_copy_mas_c_dpan

#define _op_copy_mas_c_dpas _op_copy_mas_c_dp
#define _op_copy_mas_cn_dpas _op_copy_mas_cn_dp
#define _op_copy_mas_can_dpas _op_copy_mas_can_dp
#define _op_copy_mas_caa_dpas _op_copy_mas_caa_dp

static void
init_copy_mask_color_span_funcs_c(void)
{
op_copy_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_copy_mas_cn_dp;
op_copy_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_copy_mas_c_dp;
op_copy_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_copy_mas_can_dp;
op_copy_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_copy_mas_caa_dp;

op_copy_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_mas_cn_dpan;
op_copy_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_copy_mas_c_dpan;
op_copy_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_copy_mas_can_dpan;
op_copy_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_copy_mas_caa_dpan;

op_copy_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_mas_cn_dpas;
op_copy_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_copy_mas_c_dpas;
op_copy_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_copy_mas_can_dpas;
op_copy_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_copy_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   DATA32 aa = 1 + _evas_pow_lut[(m << 8) + (*d >> 24)];
   *d = BLEND_ARGB_256(m + 1, aa, c, *d);
}

static void
_op_copy_pt_mas_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   *d = INTERP_256(m + 1, c, *d);
}


#define _op_copy_pt_mas_cn_dp _op_copy_pt_mas_c_dp
#define _op_copy_pt_mas_can_dp _op_copy_pt_mas_c_dp
#define _op_copy_pt_mas_caa_dp _op_copy_pt_mas_c_dp

#define _op_copy_pt_mas_cn_dpan _op_copy_pt_mas_c_dpan
#define _op_copy_pt_mas_can_dpan _op_copy_pt_mas_c_dpan
#define _op_copy_pt_mas_caa_dpan _op_copy_pt_mas_c_dpan

#define _op_copy_pt_mas_c_dpas _op_copy_pt_mas_c_dp
#define _op_copy_pt_mas_cn_dpas _op_copy_pt_mas_cn_dp
#define _op_copy_pt_mas_can_dpas _op_copy_pt_mas_can_dp
#define _op_copy_pt_mas_caa_dpas _op_copy_pt_mas_caa_dp

static void
init_copy_mask_color_pt_funcs_c(void)
{
op_copy_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_copy_pt_mas_cn_dp;
op_copy_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_copy_pt_mas_c_dp;
op_copy_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_copy_pt_mas_can_dp;
op_copy_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_copy_pt_mas_caa_dp;

op_copy_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_pt_mas_cn_dpan;
op_copy_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_copy_pt_mas_c_dpan;
op_copy_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_copy_pt_mas_can_dpan;
op_copy_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_copy_pt_mas_caa_dpan;

op_copy_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_pt_mas_cn_dpas;
op_copy_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_copy_pt_mas_c_dpas;
op_copy_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_copy_pt_mas_can_dpas;
op_copy_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_copy_pt_mas_caa_dpas;
}
#endif

/*-----*/

/* copy_rel mask x color -> dst */

#ifdef BUILD_C
static void
_op_copy_rel_mas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   c &= 0x00ffffff;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL_A_256(ca, *d) + c;
		break;
	    default:
	      {
		DATA32 da = 256 - (((257 - ca) * a) >> 8);
		a++;
		*d = MUL_A_256(da, *d) + INTERP_RGB_256(a, c, *d);
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_copy_rel_mas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c &= 0x00ffffff;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = (*d & 0xff000000) + c;
		break;
	    default:
		a++;
		*d = (*d & 0xff000000) + INTERP_RGB_256(a, c, *d);
		break;
	  }
	m++;  d++;
     }
}

#define _op_copy_rel_mas_cn_dp _op_copy_rel_mas_can_dp
#define _op_copy_rel_mas_caa_dp _op_copy_rel_mas_c_dp

#define _op_copy_rel_mas_c_dpan _op_copy_mas_c_dpan
#define _op_copy_rel_mas_cn_dpan _op_copy_mas_cn_dpan
#define _op_copy_rel_mas_can_dpan _op_copy_mas_can_dpan
#define _op_copy_rel_mas_caa_dpan _op_copy_mas_caa_dpan

#define _op_copy_rel_mas_c_dpas _op_copy_rel_mas_c_dp
#define _op_copy_rel_mas_cn_dpas _op_copy_rel_mas_cn_dp
#define _op_copy_rel_mas_can_dpas _op_copy_rel_mas_can_dp
#define _op_copy_rel_mas_caa_dpas _op_copy_rel_mas_caa_dp

static void
init_copy_rel_mask_color_span_funcs_c(void)
{
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_mas_cn_dp;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_copy_rel_mas_c_dp;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_copy_rel_mas_can_dp;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_copy_rel_mas_caa_dp;

   op_copy_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_mas_cn_dpan;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_copy_rel_mas_c_dpan;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_copy_rel_mas_can_dpan;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_copy_rel_mas_caa_dpan;

   op_copy_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_mas_cn_dpas;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_copy_rel_mas_c_dpas;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_copy_rel_mas_can_dpas;
   op_copy_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_copy_rel_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_rel_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   DATA32 da = 256 - (((256 - (c >> 24)) * m) >> 8);
   *d = MUL_A_256(da, *d) + INTERP_RGB_256(m + 1, c & 0x00ffffff, *d);
}

static void
_op_copy_rel_pt_mas_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   *d = (*d & 0xff000000) + INTERP_RGB_256(m + 1, c & 0x00ffffff, *d);
}

#define _op_copy_rel_pt_mas_cn_dp _op_copy_rel_pt_mas_can_dp
#define _op_copy_rel_pt_mas_caa_dp _op_copy_rel_pt_mas_c_dp

#define _op_copy_rel_pt_mas_c_dpan _op_copy_pt_mas_c_dpan
#define _op_copy_rel_pt_mas_cn_dpan _op_copy_pt_mas_cn_dpan
#define _op_copy_rel_pt_mas_can_dpan _op_copy_pt_mas_can_dpan
#define _op_copy_rel_pt_mas_caa_dpan _op_copy_pt_mas_caa_dpan

#define _op_copy_rel_pt_mas_c_dpas _op_copy_rel_pt_mas_c_dp
#define _op_copy_rel_pt_mas_cn_dpas _op_copy_rel_pt_mas_cn_dp
#define _op_copy_rel_pt_mas_can_dpas _op_copy_rel_pt_mas_can_dp
#define _op_copy_rel_pt_mas_caa_dpas _op_copy_rel_pt_mas_caa_dp

static void
init_copy_rel_mask_color_pt_funcs_c(void)
{
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_pt_mas_cn_dp;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_copy_rel_pt_mas_c_dp;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_copy_rel_pt_mas_can_dp;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_copy_rel_pt_mas_caa_dp;

   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_mas_cn_dpan;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_copy_rel_pt_mas_c_dpan;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_mas_can_dpan;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_mas_caa_dpan;

   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_mas_cn_dpas;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_copy_rel_pt_mas_c_dpas;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_copy_rel_pt_mas_can_dpas;
   op_copy_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_copy_rel_pt_mas_caa_dpas;
}
#endif
