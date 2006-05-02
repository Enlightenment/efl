
/* mask pixel --> dst */

#ifdef BUILD_C
static void
_op_mask_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
	*d = MUL_A_SYM(*s >> 24, *d) + (*d & 0x00ffffff);
   }
}

static void
_op_mask_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
      switch(*s & 0xff000000)
	 {
	   case 0:
	     *d &= 0x00ffffff;
	   break;
	   case 0xff000000:
	   break;
	   default:
	     *d = MUL_A_SYM(*s >> 24, *d) + (*d & 0x00ffffff);
	   break;
	 }
   }
}

static void
_op_mask_p_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
	*d &= (*s | 0x00ffffff);
   }
}

static void
_op_mask_pas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
      switch(*s & 0xff000000)
	 {
	   case 0:
	     *d &= 0x00ffffff;
	   break;
	   case 0xff000000:
	   break;
	   default:
	     *d &= (*s | 0x00ffffff);
	   break;
	 }
   }
}

#define _op_mask_pan_dp NULL

#define _op_mask_pan_dpan _op_mask_pan_dp

#define _op_mask_p_dpas _op_mask_p_dp
#define _op_mask_pan_dpas _op_mask_pan_dp
#define _op_mask_pas_dpas _op_mask_pas_dp

static void
init_mask_pixel_span_funcs_c(void)
{
   op_mask_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_mask_p_dp;
   op_mask_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_mask_pan_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_mask_pas_dp;

   op_mask_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_p_dpan;
   op_mask_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pan_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pas_dpan;

   op_mask_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_p_dpas;
   op_mask_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_pan_dpas;
   op_mask_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_pas_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(s >> 24, *d) + (*d & 0x00ffffff);
}

static void
_op_mask_pt_p_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d &= (s | 0x00ffffff);
}

#define _op_mask_pt_pan_dp NULL
#define _op_mask_pt_pas_dp _op_mask_pt_p_dp

#define _op_mask_pt_pan_dpan _op_mask_pt_pan_dp
#define _op_mask_pt_pas_dpan _op_mask_pt_pas_dp

#define _op_mask_pt_p_dpas _op_mask_pt_p_dp
#define _op_mask_pt_pan_dpas _op_mask_pt_pan_dp
#define _op_mask_pt_pas_dpas _op_mask_pt_pas_dp

static void
init_mask_pixel_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_mask_pt_p_dp;
   op_mask_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_mask_pt_pan_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_mask_pt_pas_dp;

   op_mask_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pt_p_dpan;
   op_mask_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pt_pan_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_mask_pt_pas_dpan;

   op_mask_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_pt_p_dpas;
   op_mask_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_pt_pan_dpas;
   op_mask_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_mask_pt_pas_dpas;
}
#endif
