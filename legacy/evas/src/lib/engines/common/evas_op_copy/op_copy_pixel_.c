
/* copy pixel --> dst */

#ifdef BUILD_C
static void
_op_copy_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
      *d = *s;
   }
}

#define _op_copy_pan_dp _op_copy_p_dp
#define _op_copy_pas_dp _op_copy_p_dp

#define _op_copy_p_dpan _op_copy_p_dp
#define _op_copy_pan_dpan _op_copy_pan_dp
#define _op_copy_pas_dpan _op_copy_pas_dp

#define _op_copy_p_dpas _op_copy_p_dp
#define _op_copy_pan_dpas _op_copy_pan_dp
#define _op_copy_pas_dpas _op_copy_pas_dp

static void
init_copy_pixel_span_funcs_c(void)
{
   op_copy_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_p_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_pan_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_pas_dp;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_p_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pan_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pas_dpan;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_p_dpas;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_pan_dpas;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_pas_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = s;
}

#define _op_copy_pt_pan_dp _op_copy_pt_p_dp
#define _op_copy_pt_pas_dp _op_copy_pt_p_dp

#define _op_copy_pt_p_dpan _op_copy_pt_p_dp
#define _op_copy_pt_pan_dpan _op_copy_pt_pan_dp
#define _op_copy_pt_pas_dpan _op_copy_pt_pas_dp

#define _op_copy_pt_p_dpas _op_copy_pt_p_dp
#define _op_copy_pt_pan_dpas _op_copy_pt_pan_dp
#define _op_copy_pt_pas_dpas _op_copy_pt_pas_dp

static void
init_copy_pixel_pt_funcs_c(void)
{
   op_copy_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_pt_p_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_pt_pan_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_pt_pas_dp;

   op_copy_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pt_p_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pt_pan_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pt_pas_dpan;

   op_copy_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_pt_p_dpas;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_pt_pan_dpas;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_pt_pas_dpas;
}
#endif

/*-----*/

/* copy_rel pixel --> dst */

#ifdef BUILD_C
static void
_op_copy_rel_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
	*d = MUL_A_SYM(*s >> 24, *d) + (*s & 0x00ffffff);
   }
}

static void
_op_copy_rel_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
      switch(*s & 0xff000000)
	 {
	   case 0:
	     *d = *s;
	   break;
	   case 0xff000000:
	     *d = (*d & 0xff000000) + (*s & 0x00ffffff);
	   break;
	   default:
	     *d = MUL_A_SYM(*s >> 24, *d) + (*s & 0x00ffffff);
	   break;
	 }
   }
}

static void
_op_copy_rel_pan_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   for (; d < e; d++, s++) {
	*d = (*d & 0xff000000) + (*s & 0x00ffffff);
   }
}

#define _op_copy_rel_p_dpan _op_copy_p_dpan
#define _op_copy_rel_pan_dpan _op_copy_pan_dpan
#define _op_copy_rel_pas_dpan _op_copy_pas_dpan

#define _op_copy_rel_p_dpas _op_copy_rel_p_dp
#define _op_copy_rel_pan_dpas _op_copy_rel_pan_dp
#define _op_copy_rel_pas_dpas _op_copy_rel_pas_dp

static void
init_copy_rel_pixel_span_funcs_c(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_p_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pan_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pas_dp;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_p_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pan_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pas_dpan;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_p_dpas;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_pan_dpas;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_pas_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_rel_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(s >> 24, *d) + (s & 0x00ffffff);
}


static void
_op_copy_rel_pt_pan_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d & 0xff000000) + (s & 0x00ffffff);
}

#define _op_copy_rel_pt_pas_dp _op_copy_rel_pt_p_dp

#define _op_copy_rel_pt_p_dpan _op_copy_pt_p_dpan
#define _op_copy_rel_pt_pan_dpan _op_copy_pt_pan_dpan
#define _op_copy_rel_pt_pas_dpan _op_copy_pt_pas_dpan

#define _op_copy_rel_pt_p_dpas _op_copy_rel_pt_p_dp
#define _op_copy_rel_pt_pan_dpas _op_copy_rel_pt_pan_dp
#define _op_copy_rel_pt_pas_dpas _op_copy_rel_pt_pas_dp

static void
init_copy_rel_pixel_pt_funcs_c(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pt_p_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pt_pan_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pt_pas_dp;

   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_p_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_pan_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_pas_dpan;

   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_p_dpas;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_pan_dpas;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_pas_dpas;
}
#endif
