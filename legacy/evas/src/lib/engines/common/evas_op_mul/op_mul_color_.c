
/* mul color --> dst */

#ifdef BUILD_C
static void
_op_mul_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++) {
      *d = MUL4_SYM(c, *d);
   }
}

static void
_op_mul_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l){
   DATA32 *e = d + l;
   for (; d < e; d++) {
      *d = (*d & 0xff000000) + MUL3_SYM(c, *d);
   }
}

static void
_op_mul_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   for (; d < e; d++) {
      *d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
   }
}

static void
_op_mul_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++) {
      *d = (c & 0xff000000) + MUL3_SYM(c, *d);
   }
}

static void
_op_mul_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l){
   DATA32 *e = d + l;
   for (; d < e; d++) {
      *d = 0xff000000 + MUL3_SYM(c, *d);
   }
}

static void
_op_mul_caa_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c &= 0xff000000;
   for (; d < e; d++) {
      *d = c + (*d & 0x00ffffff);
   }
}

#define _op_mul_c_dpas _op_mul_c_dp
#define _op_mul_can_dpas _op_mul_can_dp
#define _op_mul_caa_dpas _op_mul_caa_dp

static void
init_mul_color_span_funcs_c(void)
{
   op_mul_span_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mul_c_dp;
   op_mul_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_mul_can_dp;
   op_mul_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mul_caa_dp;

   op_mul_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mul_c_dpan;
   op_mul_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_can_dpan;
   op_mul_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_caa_dpan;

   op_mul_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_mul_c_dpas;
   op_mul_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_mul_can_dpas;
   op_mul_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_mul_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_mul_pt_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = MUL4_SYM(c, *d);
}

static void
_op_mul_pt_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d){
      *d = (*d & 0xff000000) + MUL3_SYM(c, *d);
}

static void
_op_mul_pt_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = MUL_A_SYM(c >> 24, *d) + (*d & 0x00ffffff);
}

static void
_op_mul_pt_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = (c & 0xff000000) + MUL3_SYM(c, *d);
}

static void
_op_mul_pt_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d){
      *d = 0xff000000 + MUL3_SYM(c, *d);
}

static void
_op_mul_pt_caa_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = (c & 0xff000000) + (*d & 0x00ffffff);
}

#define _op_mul_pt_c_dpas _op_mul_pt_c_dp
#define _op_mul_pt_can_dpas _op_mul_pt_can_dp
#define _op_mul_pt_caa_dpas _op_mul_pt_caa_dp

static void
init_mul_color_pt_funcs_c(void)
{
   op_mul_pt_funcs[SP_N][SM_N][SC][DP][CPU_C] = _op_mul_pt_c_dp;
   op_mul_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_C] = _op_mul_pt_can_dp;
   op_mul_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_C] = _op_mul_pt_caa_dp;

   op_mul_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_C] = _op_mul_pt_c_dpan;
   op_mul_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_C] = _op_mul_pt_can_dpan;
   op_mul_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_C] = _op_mul_pt_caa_dpan;

   op_mul_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_C] = _op_mul_pt_c_dpas;
   op_mul_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_C] = _op_mul_pt_can_dpas;
   op_mul_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_C] = _op_mul_pt_caa_dpas;
}
#endif

