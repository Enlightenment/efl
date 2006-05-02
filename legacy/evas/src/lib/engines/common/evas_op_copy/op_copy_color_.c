
/* copy color --> dst */

#ifdef BUILD_C
static void
_op_copy_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++) {
      *d = c;
   }
}

#define _op_copy_cn_dp _op_copy_c_dp
#define _op_copy_can_dp _op_copy_c_dp
#define _op_copy_caa_dp _op_copy_c_dp

#define _op_copy_c_dpan _op_copy_c_dp
#define _op_copy_cn_dpan _op_copy_c_dp
#define _op_copy_can_dpan _op_copy_c_dp
#define _op_copy_caa_dpan _op_copy_c_dp

#define _op_copy_c_dpas _op_copy_c_dp
#define _op_copy_cn_dpas _op_copy_c_dp
#define _op_copy_can_dpas _op_copy_c_dp
#define _op_copy_caa_dpas _op_copy_c_dp

static void
init_copy_color_span_funcs_c(void)
{
   op_copy_span_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_copy_cn_dp;
   op_copy_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_copy_c_dp;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_copy_can_dp;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_copy_caa_dp;

   op_copy_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_cn_dpan;
   op_copy_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_copy_c_dpan;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_can_dpan;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_caa_dpan;

   op_copy_span_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_cn_dpas;
   op_copy_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_copy_c_dpas;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_can_dpas;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = c;
}

#define _op_copy_pt_cn_dp _op_copy_pt_c_dp
#define _op_copy_pt_can_dp _op_copy_pt_c_dp
#define _op_copy_pt_caa_dp _op_copy_pt_c_dp

#define _op_copy_pt_c_dpan _op_copy_pt_c_dp
#define _op_copy_pt_cn_dpan _op_copy_pt_c_dp
#define _op_copy_pt_can_dpan _op_copy_pt_c_dp
#define _op_copy_pt_caa_dpan _op_copy_pt_c_dp

#define _op_copy_pt_c_dpas _op_copy_pt_c_dp
#define _op_copy_pt_cn_dpas _op_copy_pt_c_dp
#define _op_copy_pt_can_dpas _op_copy_pt_c_dp
#define _op_copy_pt_caa_dpas _op_copy_pt_c_dp

static void
init_copy_color_pt_funcs_c(void)
{
   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_copy_pt_cn_dp;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_copy_pt_c_dp;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_copy_pt_can_dp;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_copy_pt_caa_dp;

   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pt_cn_dpan;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_copy_pt_c_dpan;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pt_can_dpan;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pt_caa_dpan;

   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_pt_cn_dpas;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_copy_pt_c_dpas;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_pt_can_dpas;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_pt_caa_dpas;
}
#endif

/*-----*/

/* copy_rel color --> dst */

#ifdef BUILD_C
static void
_op_copy_rel_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   c &= 0x00ffffff;
   for (; d < e; d++) {
	*d = MUL_A_256(ca, *d) + c;
   }
}

static void
_op_copy_rel_cn_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++) {
	*d = (*d | 0x00ffffff);
   }
}

static void
_op_copy_rel_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c &= 0x00ffffff;
   for (; d < e; d++) {
	*d = (*d & 0xff000000) + c;
   }
}


#define _op_copy_rel_caa_dp _op_copy_rel_c_dp

#define _op_copy_rel_c_dpan _op_copy_c_dp
#define _op_copy_rel_cn_dpan _op_copy_cn_dp
#define _op_copy_rel_can_dpan _op_copy_can_dp
#define _op_copy_rel_caa_dpan _op_copy_caa_dp

#define _op_copy_rel_c_dpas _op_copy_rel_c_dp
#define _op_copy_rel_cn_dpas _op_copy_rel_cn_dp
#define _op_copy_rel_can_dpas _op_copy_rel_can_dp
#define _op_copy_rel_caa_dpas _op_copy_rel_caa_dp

static void
init_copy_rel_color_span_funcs_c(void)
{
   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_cn_dp;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_copy_rel_c_dp;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_can_dp;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_caa_dp;

   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_cn_dpan;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_c_dpan;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_can_dpan;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_caa_dpan;

   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_cn_dpas;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_c_dpas;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_can_dpas;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_rel_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(c, *d) + (c & 0x00ffffff);
}

static void
_op_copy_rel_pt_cn_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d | 0x00ffffff);
}

static void
_op_copy_rel_pt_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d & 0xff000000) + (c & 0x00ffffff);
}


#define _op_copy_rel_pt_caa_dp _op_copy_rel_pt_c_dp

#define _op_copy_rel_pt_c_dpan _op_copy_pt_c_dp
#define _op_copy_rel_pt_cn_dpan _op_copy_pt_cn_dp
#define _op_copy_rel_pt_can_dpan _op_copy_pt_can_dp
#define _op_copy_rel_pt_caa_dpan _op_copy_pt_caa_dp

#define _op_copy_rel_pt_c_dpas _op_copy_rel_pt_c_dp
#define _op_copy_rel_pt_cn_dpas _op_copy_rel_pt_cn_dp
#define _op_copy_rel_pt_can_dpas _op_copy_rel_pt_can_dp
#define _op_copy_rel_pt_caa_dpas _op_copy_rel_pt_caa_dp

static void
init_copy_rel_color_pt_funcs_c(void)
{
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pt_cn_dp;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_copy_rel_pt_c_dp;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pt_can_dp;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pt_caa_dp;

   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_cn_dpan;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pt_c_dpan;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_can_dpan;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_caa_dpan;

   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_cn_dpas;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_pt_c_dpas;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_pt_can_dpas;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_pt_caa_dpas;
}
#endif
