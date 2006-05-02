
/* mask mask x color -> dst */

#ifdef BUILD_C
static void
_op_mask_mas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
		break;
	    default:
		a = 256 - (((257 - c) * a) >> 8);
		*d = MUL_A_256(a, *d) + (*d & 0x00ffffff);
		break;
	  }
	m++;  d++;
     }
}

static void
_op_mask_mas_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c &= 0xff000000;
   while (d < e)
     {
	switch(*m)
	  {
	    case 0:
		break;
	    case 255:
		*d = c + (*d & 0x00ffffff);
		break;
	    default:
		*d = ((0xff000000 - ((*m + 1) * (0xff0000 - (c >> 8)))) & 0xff000000)
		     + (*d & 0x00ffffff);
		break;
	  }
	m++;  d++;
     }
}

#define _op_mask_mas_cn_dp NULL
#define _op_mask_mas_can_dp _op_mask_mas_cn_dp
#define _op_mask_mas_caa_dp _op_mask_mas_c_dp

#define _op_mask_mas_cn_dpan _op_mask_mas_cn_dp
#define _op_mask_mas_can_dpan _op_mask_mas_cn_dpan
#define _op_mask_mas_caa_dpan _op_mask_mas_c_dpan

#define _op_mask_mas_c_dpas _op_mask_mas_c_dp
#define _op_mask_mas_cn_dpas _op_mask_mas_cn_dp
#define _op_mask_mas_can_dpas _op_mask_mas_can_dp
#define _op_mask_mas_caa_dpas _op_mask_mas_caa_dp

static void
init_mask_mask_color_span_funcs_c(void)
{
   op_mask_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_mask_mas_cn_dp;
   op_mask_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mask_mas_c_dp;
   op_mask_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_mask_mas_can_dp;
   op_mask_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mask_mas_caa_dp;

   op_mask_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_mas_cn_dpan;
   op_mask_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mask_mas_c_dpan;
   op_mask_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_mask_mas_can_dpan;
   op_mask_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mask_mas_caa_dpan;

   op_mask_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_mas_cn_dpas;
   op_mask_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_mask_mas_c_dpas;
   op_mask_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_mask_mas_can_dpas;
   op_mask_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_mask_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 256 - (((256 - (c >> 24)) * m) >> 8);
	*d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
}

static void
_op_mask_pt_mas_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = (c & 0xff000000) >> 8;
	*d = ((0xff000000 - ((m + 1) * (0xff0000 - c))) & 0xff000000)
	     + (*d & 0x00ffffff);
}

#define _op_mask_pt_mas_cn_dp NULL
#define _op_mask_pt_mas_can_dp _op_mask_pt_mas_cn_dp
#define _op_mask_pt_mas_caa_dp _op_mask_pt_mas_c_dp

#define _op_mask_pt_mas_cn_dpan _op_mask_pt_mas_cn_dp
#define _op_mask_pt_mas_can_dpan _op_mask_pt_mas_cn_dpan
#define _op_mask_pt_mas_caa_dpan _op_mask_pt_mas_c_dpan

#define _op_mask_pt_mas_c_dpas _op_mask_pt_mas_c_dp
#define _op_mask_pt_mas_cn_dpas _op_mask_pt_mas_cn_dp
#define _op_mask_pt_mas_can_dpas _op_mask_pt_mas_can_dp
#define _op_mask_pt_mas_caa_dpas _op_mask_pt_mas_caa_dp

static void
init_mask_mask_color_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_mask_pt_mas_cn_dp;
   op_mask_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mask_pt_mas_c_dp;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_mask_pt_mas_can_dp;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mask_pt_mas_caa_dp;

   op_mask_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pt_mas_cn_dpan;
   op_mask_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mask_pt_mas_c_dpan;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_mask_pt_mas_can_dpan;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mask_pt_mas_caa_dpan;

   op_mask_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_pt_mas_cn_dpas;
   op_mask_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_mask_pt_mas_c_dpas;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_mask_pt_mas_can_dpas;
   op_mask_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_mask_pt_mas_caa_dpas;
}
#endif
