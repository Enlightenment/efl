
/* add mask x color -> dst */

#ifdef BUILD_C
static void
_op_add_mas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, cc = c & 0x00ffffff;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
	      {
		DATA32 caa = 1 + _evas_pow_lut[((c >> 16) & 0xff00) + (*d >> 24)];
		DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (c >> 24)];
		MUL2_ADD_256(caa, daa, c, *d, *d)
	      }
		break;
	    default:
	      {
		DATA32 caa, daa;
		a = MUL_A_SYM(a, c) + cc;
		caa = 1 + _evas_pow_lut[((a >> 16) & 0xff00) + (*d >> 24)];
		daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (a >> 24)];
		MUL2_ADD_256(caa, daa, a, *d, *d)
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_add_mas_cn_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = 0xffffffff;
		break;
	    default:
	      {
		DATA32 caa, daa;
		a = (*m << 24) + 0x00ffffff;
		caa = 1 + _evas_pow_lut[(*m << 8) + (*d >> 24)];
		daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + *m];
		MUL2_ADD_256(caa, daa, a, *d, *d)
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_add_mas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
		MUL_ADD_SYM(*d >> 24, *d, c, *d)
		break;
	    default:
	      {
		DATA32 caa, daa;
		a = (a << 24) + c;
		caa = 1 + _evas_pow_lut[((a >> 16) & 0xff00) + (*d >> 24)];
		daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (a >> 24)];
		MUL2_ADD_256(caa, daa, a, *d, *d)
	      }
		break;
	  }
	m++;  d++;
     }
}

static void
_op_add_mas_cn_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = 0xffffffff;
		break;
	    default:
		a = a * 0x00010101;
		ADD(a, *d, *d)
		break;
	  }
	m++;  d++;
     }
}

static void
_op_add_mas_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   DATA32 ac = MUL_256(ca, c) & 0x00ffffff;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		ADD(ac, *d, *d)
		break;
	    default:
		a = 1 + ((a * ca) >> 8);
		MUL_ADD_256(a, c, *d, *d)
		break;
	  }
	m++;  d++;
     }
}

static void
_op_add_mas_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*m)
	  {
	    case 0:
		break;
	    case 255:
		ADD(c, *d, *d)
		break;
	    default:
		MUL_ADD_SYM(*m, c, *d, *d)
		break;
	  }
	m++;  d++;
     }
}

#define _op_add_mas_caa_dp _op_add_mas_c_dp

#define _op_add_mas_caa_dpan _op_add_mas_c_dpan

#define _op_add_mas_c_dpas _op_add_mas_c_dp
#define _op_add_mas_cn_dpas _op_add_mas_cn_dp
#define _op_add_mas_can_dpas _op_add_mas_can_dp
#define _op_add_mas_caa_dpas _op_add_mas_caa_dp

static void
init_add_mask_color_span_funcs_c(void)
{
   op_add_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_add_mas_c_dp;
   op_add_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_add_mas_cn_dp;
   op_add_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_add_mas_can_dp;
   op_add_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_add_mas_caa_dp;

   op_add_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_add_mas_c_dpan;
   op_add_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_mas_cn_dpan;
   op_add_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_add_mas_can_dpan;
   op_add_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_add_mas_caa_dpan;

   op_add_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_add_mas_c_dpas;
   op_add_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_mas_cn_dpas;
   op_add_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_add_mas_can_dpas;
   op_add_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_add_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_add_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 caa, daa;
	c = MUL_A_SYM(m, c) + (c & 0x00ffffff);
	caa = 1 + _evas_pow_lut[((c >> 16) & 0xff00) + (*d >> 24)];
	daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (c >> 24)];
	MUL2_ADD_256(caa, daa, c, *d, *d)
}

static void
_op_add_pt_mas_cn_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 caa, daa;
	c = (m << 24) + 0x00ffffff;
	caa = 1 + _evas_pow_lut[(m << 8) + (*d >> 24)];
	daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + m];
	MUL2_ADD_256(caa, daa, c, *d, *d)
}

static void
_op_add_pt_mas_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 caa, daa;
	c = (m << 24) + (c & 0x00ffffff);
	caa = 1 + _evas_pow_lut[(m << 8) + (*d >> 24)];
	daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + m];
	MUL2_ADD_256(caa, daa, c, *d, *d)
}

static void
_op_add_pt_mas_cn_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = (m << 24) + 0x00ffffff;
	ADD(c, *d, *d)
}

static void
_op_add_pt_mas_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 1 + (((m * (c >> 24)) + 255) >> 8);
	MUL_ADD_256(s, c, *d, *d)
}

static void
_op_add_pt_mas_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_ADD_SYM(m, c, *d, *d)
}

#define _op_add_pt_mas_caa_dp _op_add_pt_mas_c_dp

#define _op_add_pt_mas_caa_dpan _op_add_pt_mas_c_dpan

#define _op_add_pt_mas_c_dpas _op_add_pt_mas_c_dp
#define _op_add_pt_mas_cn_dpas _op_add_pt_mas_cn_dp
#define _op_add_pt_mas_can_dpas _op_add_pt_mas_can_dp
#define _op_add_pt_mas_caa_dpas _op_add_pt_mas_caa_dp

static void
init_add_mask_color_pt_funcs_c(void)
{
   op_add_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_add_pt_mas_c_dp;
   op_add_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_add_pt_mas_cn_dp;
   op_add_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_add_pt_mas_can_dp;
   op_add_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_add_pt_mas_caa_dp;

   op_add_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_add_pt_mas_c_dpan;
   op_add_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_pt_mas_cn_dpan;
   op_add_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_add_pt_mas_can_dpan;
   op_add_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_add_pt_mas_caa_dpan;

   op_add_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_add_pt_mas_c_dpas;
   op_add_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_pt_mas_cn_dpas;
   op_add_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_add_pt_mas_can_dpas;
   op_add_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_add_pt_mas_caa_dpas;
}
#endif

/*-----*/

/* add_rel mask x color -> dst */

#ifdef BUILD_C
static void
_op_add_rel_mas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   DATA32 ac = MUL_256(ca, c) & 0x00ffffff;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		ADD_REL(ac, *d, *d)
		break;
	    default:
		a = 1 + ((a * ca) >> 8);
		MUL_ADD_REL_256(a, c, *d, *d)
		break;
	  }
	m++;  d++;
     }
}

static void
_op_add_rel_mas_cn_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d |= 0x00ffffff;
		break;
	    default:
		a = a * 0x00010101;
		ADD_REL(a, *d, *d)
		break;
	  }
	m++;  d++;
     }
}

static void
_op_add_rel_mas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*m)
	  {
	    case 0:
		break;
	    case 255:
		ADD_REL(c, *d, *d)
		break;
	    default:
		MUL_ADD_REL_SYM(*m, c, *d, *d)
		break;
	  }
	m++;  d++;
     }
}

#define _op_add_rel_mas_caa_dp _op_add_rel_mas_c_dp

#define _op_add_rel_mas_c_dpan _op_add_mas_c_dpan
#define _op_add_rel_mas_cn_dpan _op_add_mas_cn_dpan
#define _op_add_rel_mas_can_dpan _op_add_mas_can_dpan
#define _op_add_rel_mas_caa_dpan _op_add_mas_caa_dpan

#define _op_add_rel_mas_c_dpas _op_add_rel_mas_c_dp
#define _op_add_rel_mas_cn_dpas _op_add_rel_mas_cn_dp
#define _op_add_rel_mas_can_dpas _op_add_rel_mas_can_dp
#define _op_add_rel_mas_caa_dpas _op_add_rel_mas_caa_dp

static void
init_add_rel_mask_color_span_funcs_c(void)
{
   op_add_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_add_rel_mas_c_dp;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_mas_cn_dp;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_add_rel_mas_can_dp;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_add_rel_mas_caa_dp;

   op_add_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_add_rel_mas_c_dpan;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_mas_cn_dpan;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_add_rel_mas_can_dpan;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_add_rel_mas_caa_dpan;

   op_add_rel_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_add_rel_mas_c_dpas;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_mas_cn_dpas;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_add_rel_mas_can_dpas;
   op_add_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_add_rel_mas_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_add_rel_pt_mas_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 1 + (((m * (c >> 24)) + 255) >> 8);
	MUL_ADD_REL_256(s, c, *d, *d)
}

static void
_op_add_rel_pt_mas_cn_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = (m << 24) + 0x00ffffff;
	ADD_REL(c, *d, *d)
}

static void
_op_add_rel_pt_mas_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_ADD_REL_SYM(m, c, *d, *d)
}

#define _op_add_rel_pt_mas_caa_dp _op_add_rel_pt_mas_c_dp

#define _op_add_rel_pt_mas_c_dpan _op_add_pt_mas_c_dpan
#define _op_add_rel_pt_mas_cn_dpan _op_add_pt_mas_cn_dpan
#define _op_add_rel_pt_mas_can_dpan _op_add_pt_mas_can_dpan
#define _op_add_rel_pt_mas_caa_dpan _op_add_pt_mas_caa_dpan

#define _op_add_rel_pt_mas_c_dpas _op_add_rel_pt_mas_c_dp
#define _op_add_rel_pt_mas_cn_dpas _op_add_rel_pt_mas_cn_dp
#define _op_add_rel_pt_mas_can_dpas _op_add_rel_pt_mas_can_dp
#define _op_add_rel_pt_mas_caa_dpas _op_add_rel_pt_mas_caa_dp

static void
init_add_rel_mask_color_pt_funcs_c(void)
{
   op_add_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_C] = _op_add_rel_pt_mas_c_dp;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_pt_mas_cn_dp;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_C] = _op_add_rel_pt_mas_can_dp;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_C] = _op_add_rel_pt_mas_caa_dp;

   op_add_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_C] = _op_add_rel_pt_mas_c_dpan;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_pt_mas_cn_dpan;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_C] = _op_add_rel_pt_mas_can_dpan;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_C] = _op_add_rel_pt_mas_caa_dpan;

   op_add_rel_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_C] = _op_add_rel_pt_mas_c_dpas;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_pt_mas_cn_dpas;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_C] = _op_add_rel_pt_mas_can_dpas;
   op_add_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_C] = _op_add_rel_pt_mas_caa_dpas;
}
#endif
