
/* sub color -> dst */

#ifdef BUILD_C
static void
_op_sub_cn_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
	*d++ = 0xffffffff;
}

static void
_op_sub_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 caa = 1 + _evas_pow_lut[((c >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (c >> 24)];
	MUL2_SUB_256(caa, daa, c, *d, *d++)
     }
}

static void
_op_sub_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
	MUL_SUB_REV_SYM(*d >> 24, *d, c, *d++)
}

static void
_op_sub_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   c &= 0x00ffffff;
   c = MUL_256(ca, c);
   while (d < e)
	SUB(c, *d, *d++)
}

static void
_op_sub_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
	SUB(c, *d, *d++)
}

#define _op_sub_caa_dp _op_sub_c_dp

#define _op_sub_cn_dpan _op_sub_cn_dp
#define _op_sub_caa_dpan _op_sub_c_dpan

#define _op_sub_c_dpas _op_sub_c_dp
#define _op_sub_cn_dpas _op_sub_cn_dp
#define _op_sub_can_dpas _op_sub_can_dp
#define _op_sub_caa_dpas _op_sub_caa_dp

static void
init_sub_color_span_funcs_c(void)
{
   op_sub_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_sub_c_dp;
   op_sub_span_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_sub_cn_dp;
   op_sub_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_sub_can_dp;
   op_sub_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_sub_caa_dp;

   op_sub_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_sub_c_dpan;
   op_sub_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_cn_dpan;
   op_sub_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_can_dpan;
   op_sub_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_caa_dpan;

   op_sub_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_sub_c_dpas;
   op_sub_span_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_cn_dpas;
   op_sub_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_can_dpas;
   op_sub_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_sub_pt_cn_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = 0xffffffff;
}

static void
_op_sub_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 caa = 1 + _evas_pow_lut[((c >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (c >> 24)];
	MUL2_SUB_256(caa, daa, c, *d, *d)
}

static void
_op_sub_pt_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_SUB_REV_SYM(*d >> 24, *d, c, *d)
}

static void
_op_sub_pt_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 1 + (c >> 24);
	c &= 0x00ffffff;
	c = MUL_256(s, c);
	SUB(c, *d, *d)
}

static void
_op_sub_pt_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	SUB(c, *d, *d)
}

#define _op_sub_pt_caa_dp _op_sub_pt_c_dp

#define _op_sub_pt_cn_dpan _op_sub_pt_cn_dp
#define _op_sub_pt_caa_dpan _op_sub_pt_c_dpan

#define _op_sub_pt_c_dpas _op_sub_pt_c_dp
#define _op_sub_pt_cn_dpas _op_sub_pt_cn_dp
#define _op_sub_pt_can_dpas _op_sub_pt_can_dp
#define _op_sub_pt_caa_dpas _op_sub_pt_caa_dp

static void
init_sub_color_pt_funcs_c(void)
{
   op_sub_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_sub_pt_c_dp;
   op_sub_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_sub_pt_cn_dp;
   op_sub_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_sub_pt_can_dp;
   op_sub_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_sub_pt_caa_dp;

   op_sub_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_sub_pt_c_dpan;
   op_sub_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_pt_cn_dpan;
   op_sub_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_pt_can_dpan;
   op_sub_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_pt_caa_dpan;

   op_sub_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_sub_pt_c_dpas;
   op_sub_pt_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_pt_cn_dpas;
   op_sub_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_pt_can_dpas;
   op_sub_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_pt_caa_dpas;
}
#endif

/*-----*/

/* sub_rel color -> dst */

#ifdef BUILD_C
static void
_op_sub_rel_cn_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
	*d++ = *d | 0x00ffffff;
}

static void
_op_sub_rel_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   c &= 0x00ffffff;
   c = MUL_256(ca, c);
   while (d < e)
	SUB_REL(c, *d, *d++)
}

static void
_op_sub_rel_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
	SUB_REL(c, *d, *d++)
}

#define _op_sub_rel_caa_dp _op_sub_rel_c_dp

#define _op_sub_rel_cn_dpan _op_sub_cn_dpan
#define _op_sub_rel_c_dpan _op_sub_c_dpan
#define _op_sub_rel_can_dpan _op_sub_can_dpan
#define _op_sub_rel_caa_dpan _op_sub_caa_dpan

#define _op_sub_rel_cn_dpas _op_sub_rel_cn_dp
#define _op_sub_rel_c_dpas _op_sub_rel_c_dp
#define _op_sub_rel_can_dpas _op_sub_rel_can_dp
#define _op_sub_rel_caa_dpas _op_sub_rel_caa_dp

static void
init_sub_rel_color_span_funcs_c(void)
{
   op_sub_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_sub_rel_c_dp;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_cn_dp;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_can_dp;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_caa_dp;

   op_sub_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_c_dpan;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_cn_dpan;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_can_dpan;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_caa_dpan;

   op_sub_rel_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_c_dpas;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_cn_dpas;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_can_dpas;
   op_sub_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_sub_rel_pt_cn_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d |= 0x00ffffff;
}

static void
_op_sub_rel_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 1 + (c >> 24);
	c &= 0x00ffffff;
	c = MUL_256(s, c);
	SUB_REL(c, *d, *d)
}

static void
_op_sub_rel_pt_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	SUB_REL(c, *d, *d)
}

#define _op_sub_rel_pt_caa_dp _op_sub_rel_pt_c_dp

#define _op_sub_rel_pt_cn_dpan _op_sub_pt_cn_dpan
#define _op_sub_rel_pt_c_dpan _op_sub_pt_c_dpan
#define _op_sub_rel_pt_can_dpan _op_sub_pt_can_dpan
#define _op_sub_rel_pt_caa_dpan _op_sub_pt_caa_dpan

#define _op_sub_rel_pt_cn_dpas _op_sub_rel_pt_cn_dp
#define _op_sub_rel_pt_c_dpas _op_sub_rel_pt_c_dp
#define _op_sub_rel_pt_can_dpas _op_sub_rel_pt_can_dp
#define _op_sub_rel_pt_caa_dpas _op_sub_rel_pt_caa_dp

static void
init_sub_rel_color_pt_funcs_c(void)
{
   op_sub_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_sub_rel_pt_c_dp;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_pt_cn_dp;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_pt_can_dp;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_pt_caa_dp;

   op_sub_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_pt_c_dpan;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_pt_cn_dpan;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_pt_can_dpan;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_pt_caa_dpan;

   op_sub_rel_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_pt_c_dpas;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_pt_cn_dpas;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_pt_can_dpas;
   op_sub_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_pt_caa_dpas;
}
#endif
