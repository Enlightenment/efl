
/* mul mask x color -> dst */

#ifdef BUILD_C
static void
_op_mul_mas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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

static void
_op_mul_mas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, nc = ~c;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = (*d & 0xff000000) + MUL3_SYM(c, *d);
		break;
	    default:
		a = ~MUL_SYM(a, nc);
		*d = (*d & 0xff000000) + MUL3_SYM(a, *d);
		break;
	  }
	m++;  d++;
     }
}

static void
_op_mul_mas_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
		a = 256 - ((a * (257 - c)) >> 8);
		*d = MUL_A_256(a, *d) + (*d & 0x00ffffff);
		break;
	  }
	m++;  d++;
     }
}

static void
_op_mul_mas_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, nc = ~c;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = (c & 0xff000000) + MUL3_SYM(c, *d);
		break;
	    default:
		a = ~MUL_SYM(*m, nc);
		*d = (a & 0xff000000) + MUL3_SYM(a, *d);
		break;
	  }
	m++;  d++;
     }
}

static void
_op_mul_mas_caa_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, nc = ~c;
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
		*d = (0xff000000 - MUL_A_SYM(*m, nc)) + (*d & 0x00ffffff);
		break;
	  }
	m++;  d++;
     }
}

#define _op_mul_mas_cn_dp NULL

#define _op_mul_mas_cn_dpan _op_mul_mas_cn_dp
#define _op_mul_mas_can_dpan _op_mul_mas_c_dpan

#define _op_mul_mas_cn_dpas _op_mul_mas_cn_dp
#define _op_mul_mas_c_dpas _op_mul_mas_c_dp
#define _op_mul_mas_can_dpas _op_mul_mas_can_dp
#define _op_mul_mas_caa_dpas _op_mul_mas_caa_dp

static void
init_mul_mask_color_span_funcs_c(void)
{
   op_mul_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_mul_mas_cn_dp;
   op_mul_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mul_mas_c_dp;
   op_mul_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_mul_mas_can_dp;
   op_mul_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mul_mas_caa_dp;

   op_mul_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_mas_cn_dpan;
   op_mul_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mul_mas_c_dpan;
   op_mul_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_mul_mas_can_dpan;
   op_mul_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mul_mas_caa_dpan;

   op_mul_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_mul_mas_cn_dpas;
   op_mul_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_mul_mas_c_dpas;
   op_mul_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_mul_mas_can_dpas;
   op_mul_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_mul_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_mul_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = ~c;
	c = ~MUL_SYM(m, c);
	*d = MUL4_SYM(c, *d);
}

static void
_op_mul_pt_mas_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = ~c;
	c = ~MUL_SYM(m, c);
	*d = (*d & 0xff000000) + MUL3_SYM(c, *d);
}

static void
_op_mul_pt_mas_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 256 - ((m * (256 - (c >> 24))) >> 8);
	*d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
}

static void
_op_mul_pt_mas_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = ~c;
	c = ~MUL_SYM(m, c);
	*d = (c & 0xff000000) + MUL3_SYM(c, *d);
}

static void
_op_mul_pt_mas_caa_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = ~c;
	*d = (0xff000000 - MUL_A_SYM(m, c)) + (*d & 0x00ffffff);
}

#define _op_mul_pt_mas_cn_dp NULL

#define _op_mul_pt_mas_cn_dpan _op_mul_pt_mas_cn_dp
#define _op_mul_pt_mas_can_dpan _op_mul_pt_mas_c_dpan

#define _op_mul_pt_mas_cn_dpas _op_mul_pt_mas_cn_dp
#define _op_mul_pt_mas_c_dpas _op_mul_pt_mas_c_dp
#define _op_mul_pt_mas_can_dpas _op_mul_pt_mas_can_dp
#define _op_mul_pt_mas_caa_dpas _op_mul_pt_mas_caa_dp

static void
init_mul_mask_color_pt_funcs_c(void)
{
   op_mul_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_mul_pt_mas_cn_dp;
   op_mul_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_mul_pt_mas_c_dp;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_mul_pt_mas_can_dp;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_mul_pt_mas_caa_dp;

   op_mul_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_pt_mas_cn_dpan;
   op_mul_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_mul_pt_mas_c_dpan;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_mul_pt_mas_can_dpan;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_mul_pt_mas_caa_dpan;

   op_mul_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_mul_pt_mas_cn_dpas;
   op_mul_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_mul_pt_mas_c_dpas;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_mul_pt_mas_can_dpas;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_mul_pt_mas_caa_dpas;
}
#endif
