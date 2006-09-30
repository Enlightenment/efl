
/* mul pixel x mask --> dst */

#ifdef BUILD_C
static void
_op_mul_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	c = *m;
	switch(c)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL4_SYM(*s, *d);
		break;
	    default:
		c = ~(*s);
		c = ~MUL_SYM(*m, c);
		*d = MUL4_SYM(c, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_mul_pan_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	c = *m;
	switch(c)
	  {
	    case 0:
		break;
	    case 255:
		*d = (*d & 0xff000000) + MUL3_SYM(*s, *d);
		break;
	    default:
		c = ~(*s);
		c = ~MUL_SYM(*m, c);
		*d = (*d & 0xff000000) + MUL3_SYM(c, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_mul_p_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	c = *m;
	switch(c)
	  {
	    case 0:
		break;
	    case 255:
		*d = (*s & 0xff000000) + MUL3_SYM(*s, *d);
		break;
	    default:
		c = ~(*s);
		c = ~MUL_SYM(*m, c);
		*d = (c & 0xff000000) + MUL3_SYM(c, *d);
		break;
	  }
	m++;  d++;
     }
}

#define _op_mul_pas_mas_dp _op_mul_p_mas_dp

#define _op_mul_pan_mas_dpan _op_mul_p_mas_dpan
#define _op_mul_pas_mas_dpan _op_mul_p_mas_dpan

static void
init_mul_pixel_mask_span_funcs_c(void)
{
   op_mul_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_mul_p_mas_dp;
   op_mul_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_mul_pas_mas_dp;
   op_mul_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_mul_pan_mas_dp;

   op_mul_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_p_mas_dpan;
   op_mul_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_pas_mas_dpan;
   op_mul_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_pan_mas_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_mul_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = ~s;
	s = ~MUL_SYM(m, s);
	*d = MUL4_SYM(s, *d);
}

#define _op_mul_pt_pas_mas_dp _op_mul_pt_p_mas_dp
#define _op_mul_pt_pan_mas_dp _op_mul_pt_p_mas_dp

#define _op_mul_pt_p_mas_dpan _op_mul_pt_p_mas_dp
#define _op_mul_pt_pas_mas_dpan _op_mul_pt_p_mas_dp
#define _op_mul_pt_pan_mas_dpan _op_mul_pt_p_mas_dp

static void
init_mul_pixel_mask_pt_funcs_c(void)
{
   op_mul_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_mul_pt_p_mas_dp;
   op_mul_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_mul_pt_pas_mas_dp;
   op_mul_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_mul_pt_pan_mas_dp;

   op_mul_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_pt_p_mas_dpan;
   op_mul_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_pt_pas_mas_dpan;
   op_mul_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_mul_pt_pan_mas_dpan;
}
#endif
