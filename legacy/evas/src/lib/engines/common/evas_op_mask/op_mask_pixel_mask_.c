
/* mask pixel x mask --> dst */

#ifdef BUILD_C
static void 
_op_mask_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL_A_SYM(*s >> 24, *d) + (*d & 0x00ffffff);
		break;
	    default:
		a = 256 - (((256 - (*s >> 24)) * a) >> 8);
		*d = MUL_A_256(a, *d) + (*d & 0x00ffffff);
		break;
	  }
	m++;  s++;  d++;
     }
}

static void 
_op_mask_p_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*m)
	  {
	    case 0:
		break;
	    case 255:
		*d = (*s & 0xff000000) + (*d & 0x00ffffff);
		break;
	    default:
		*d = ((0xff000000 - ((1 + *m) * (0xff0000 - ((*s >> 8) & 0xff0000)))) & 0xff000000)
		     + (*d & 0x00ffffff);
		break;
	  }
	m++;  s++;  d++;
     }
}


#define _op_mask_pan_mas_dp NULL
#define _op_mask_pas_mas_dp _op_mask_p_mas_dp

#define _op_mask_pan_mas_dpan _op_mask_pan_mas_dp
#define _op_mask_pas_mas_dpan _op_mask_p_mas_dpan

#define _op_mask_p_mas_dpas _op_mask_p_mas_dp
#define _op_mask_pan_mas_dpas _op_mask_pan_mas_dp
#define _op_mask_pas_mas_dpas _op_mask_pas_mas_dp

static void
init_mask_pixel_mask_span_funcs_c(void)
{
   op_mask_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_mask_p_mas_dp;
   op_mask_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_mask_pan_mas_dp;
   op_mask_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_mask_pas_mas_dp;

   op_mask_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_p_mas_dpan;
   op_mask_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pan_mas_dpan;
   op_mask_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pas_mas_dpan;

   op_mask_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_p_mas_dpas;
   op_mask_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_pan_mas_dpas;
   op_mask_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_pas_mas_dpas;
}
#endif

#ifdef BUILD_C
static void 
_op_mask_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 256 - (((256 - (s >> 24)) * m) >> 8);
	*d = MUL_A_256(s, *d) + (*d & 0x00ffffff);
}

static void 
_op_mask_pt_p_mas_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = ((0xff000000 - ((m + 1) * (0xff0000 - ((s >> 8) & 0xff0000)))) & 0xff000000)
	     + (*d & 0x00ffffff);
}


#define _op_mask_pt_pan_mas_dp NULL
#define _op_mask_pt_pas_mas_dp _op_mask_pt_p_mas_dp

#define _op_mask_pt_pan_mas_dpan _op_mask_pt_pan_mas_dp
#define _op_mask_pt_pas_mas_dpan _op_mask_pt_p_mas_dpan

#define _op_mask_pt_p_mas_dpas _op_mask_pt_p_mas_dp
#define _op_mask_pt_pan_mas_dpas _op_mask_pt_pan_mas_dp
#define _op_mask_pt_pas_mas_dpas _op_mask_pt_pas_mas_dp

static void
init_mask_pixel_mask_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_mask_pt_p_mas_dp;
   op_mask_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_mask_pt_pan_mas_dp;
   op_mask_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_mask_pt_pas_mas_dp;

   op_mask_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pt_p_mas_dpan;
   op_mask_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pt_pan_mas_dpan;
   op_mask_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_mask_pt_pas_mas_dpan;

   op_mask_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_pt_p_mas_dpas;
   op_mask_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_pt_pan_mas_dpas;
   op_mask_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_mask_pt_pas_mas_dpas;
}
#endif
