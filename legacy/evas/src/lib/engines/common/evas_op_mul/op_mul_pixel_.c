
/* mul pixel --> dst */

#ifdef BUILD_C
static void
_op_mul_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
      *d = MUL4_SYM(*s, *d);
   }
}

static void
_op_mul_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		*d = MUL3_SYM(*s, *d);
		break;
	    case 0xff000000:
		*d = (*d & 0xff000000) + MUL3_SYM(*s, *d);
		break;
	    default:
		*d = MUL4_SYM(*s, *d);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_mul_pan_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = (*d & 0xff000000) + MUL3_SYM(*s, *d);
	s++;  d++;
     }
}

#define _op_mul_p_dpan _op_mul_p_dp
#define _op_mul_pan_dpan _op_mul_pan_dp
#define _op_mul_pas_dpan _op_mul_pas_dp

#define _op_mul_p_dpas _op_mul_p_dp
#define _op_mul_pan_dpas _op_mul_pan_dp
#define _op_mul_pas_dpas _op_mul_pas_dp

static void
init_mul_pixel_span_funcs_c(void)
{
   op_mul_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_mul_p_dp;
   op_mul_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_mul_pas_dp;
   op_mul_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_mul_pan_dp;

   op_mul_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_mul_p_dpan;
   op_mul_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_mul_pas_dpan;
   op_mul_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_mul_pan_dpan;

   op_mul_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_mul_p_dpas;
   op_mul_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_mul_pas_dpas;
   op_mul_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_mul_pan_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_mul_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = MUL4_SYM(s, *d);
}

static void
_op_mul_pt_pan_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d & 0xff000000) + MUL3_SYM(s, *d);
}

#define _op_mul_pt_pas_dp _op_mul_pt_p_dp

#define _op_mul_pt_p_dpan _op_mul_pt_p_dp
#define _op_mul_pt_pan_dpan _op_mul_pt_pan_dp
#define _op_mul_pt_pas_dpan _op_mul_pt_pas_dp

#define _op_mul_pt_p_dpas _op_mul_pt_p_dp
#define _op_mul_pt_pan_dpas _op_mul_pt_pan_dp
#define _op_mul_pt_pas_dpas _op_mul_pt_pas_dp

static void
init_mul_pixel_pt_funcs_c(void)
{
   op_mul_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_mul_pt_p_dp;
   op_mul_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_mul_pt_pas_dp;
   op_mul_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_mul_pt_pan_dp;

   op_mul_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_mul_pt_p_dpan;
   op_mul_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_mul_pt_pas_dpan;
   op_mul_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_mul_pt_pan_dpan;

   op_mul_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_mul_pt_p_dpas;
   op_mul_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_mul_pt_pas_dpas;
   op_mul_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_mul_pt_pan_dpas;
}
#endif
