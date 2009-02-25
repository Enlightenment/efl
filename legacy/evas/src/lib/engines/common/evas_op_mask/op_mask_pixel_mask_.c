
/* mask pixel x mask --> dst */

#ifdef BUILD_C
static void 
_op_mask_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL_SYM(*s >> 24, *d);
		break;
	    default:
		l = 256 - (((256 - (*s >> 24)) * l) >> 8);
		*d = MUL_256(l, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_mask_pas_mas_dp _op_mask_p_mas_dp

#define _op_mask_p_mas_dpan _op_mask_p_mas_dp
#define _op_mask_pas_mas_dpan _op_mask_pas_mas_dp

static void
init_mask_pixel_mask_span_funcs_c(void)
{
   op_mask_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_mask_p_mas_dp;
   op_mask_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_mask_pas_mas_dp;

   op_mask_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_p_mas_dpan;
   op_mask_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pas_mas_dpan;
}
#endif

#ifdef BUILD_C
static void 
_op_mask_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c __UNUSED__, DATA32 *d) {
	s = 256 - (((256 - (s >> 24)) * m) >> 8);
	*d = MUL_256(s, *d);
}

#define _op_mask_pt_pas_mas_dp _op_mask_pt_p_mas_dp

#define _op_mask_pt_p_mas_dpan _op_mask_pt_p_mas_dp
#define _op_mask_pt_pas_mas_dpan _op_mask_pt_pas_mas_dp

static void
init_mask_pixel_mask_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_mask_pt_p_mas_dp;
   op_mask_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_mask_pt_pas_mas_dp;

   op_mask_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pt_p_mas_dpan;
   op_mask_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pt_pas_mas_dpan;
}
#endif
