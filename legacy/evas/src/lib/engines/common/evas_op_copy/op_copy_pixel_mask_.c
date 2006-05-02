
/* copy pixel x mask --> dst */

#ifdef BUILD_C
static void 
_op_copy_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = *s;
		break;
	    default:
	      {
		DATA32 aa = 1 + _evas_pow_lut[(a << 8) + (*d >> 24)];
		a++;
		*d = BLEND_ARGB_256(a, aa, *s, *d);
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_copy_p_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = *s;
		break;
	    default:
		a++;
		*d = INTERP_256(a, *s, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_copy_pan_mas_dp _op_copy_p_mas_dp
#define _op_copy_pas_mas_dp _op_copy_p_mas_dp

#define _op_copy_pan_mas_dpan _op_copy_p_mas_dpan
#define _op_copy_pas_mas_dpan _op_copy_p_mas_dpan

#define _op_copy_p_mas_dpas _op_copy_p_mas_dp
#define _op_copy_pan_mas_dpas _op_copy_pan_mas_dp
#define _op_copy_pas_mas_dpas _op_copy_pas_mas_dp

static void
init_copy_pixel_mask_span_funcs_c(void)
{
   op_copy_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_copy_p_mas_dp;
   op_copy_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_copy_pan_mas_dp;
   op_copy_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_copy_pas_mas_dp;

   op_copy_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_p_mas_dpan;
   op_copy_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_pan_mas_dpan;
   op_copy_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_pas_mas_dpan;

   op_copy_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_p_mas_dpas;
   op_copy_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_pan_mas_dpas;
   op_copy_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_pas_mas_dpas;
}
#endif

#ifdef BUILD_C
static void 
_op_copy_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 aa = 1 + _evas_pow_lut[(m << 8) + (*d >> 24)];
	*d = BLEND_ARGB_256(m + 1, aa, s, *d);
}

static void
_op_copy_pt_p_mas_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = INTERP_256(m + 1, s, *d);
}

#define _op_copy_pt_pan_mas_dp _op_copy_pt_p_mas_dp
#define _op_copy_pt_pas_mas_dp _op_copy_pt_p_mas_dp

#define _op_copy_pt_pan_mas_dpan _op_copy_pt_p_mas_dpan
#define _op_copy_pt_pas_mas_dpan _op_copy_pt_p_mas_dpan

#define _op_copy_pt_p_mas_dpas _op_copy_pt_p_mas_dp
#define _op_copy_pt_pan_mas_dpas _op_copy_pt_pan_mas_dp
#define _op_copy_pt_pas_mas_dpas _op_copy_pt_pas_mas_dp

static void
init_copy_pixel_mask_pt_funcs_c(void)
{
   op_copy_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_copy_pt_p_mas_dp;
   op_copy_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_copy_pt_pan_mas_dp;
   op_copy_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_copy_pt_pas_mas_dp;

   op_copy_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_pt_p_mas_dpan;
   op_copy_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_pt_pan_mas_dpan;
   op_copy_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_pt_pas_mas_dpan;

   op_copy_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_pt_p_mas_dpas;
   op_copy_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_pt_pan_mas_dpas;
   op_copy_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_pt_pas_mas_dpas;
}
#endif

/*-----*/

/* copy_rel pixel x mask --> dst */

#ifdef BUILD_C
static void 
_op_copy_rel_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = MUL_A_SYM(*s >> 24, *d) + (*s & 0x00ffffff);
		break;
	    default:
	      {
		DATA32 da = 256 - (((256 - (*s >> 24)) * a) >> 8);
		a++;
		*d = MUL_A_256(da, *d) + INTERP_RGB_256(a, *s, *d);
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void 
_op_copy_rel_pan_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = (*d | 0x00ffffff) & *s;
		break;
	    default:
		a++;
		*d = (*d & 0xff000000) + INTERP_RGB_256(a, *s, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_copy_rel_pas_mas_dp _op_copy_rel_p_mas_dp

#define _op_copy_rel_p_mas_dpan _op_copy_p_mas_dpan
#define _op_copy_rel_pan_mas_dpan _op_copy_pan_mas_dpan
#define _op_copy_rel_pas_mas_dpan _op_copy_pas_mas_dpan

#define _op_copy_rel_p_mas_dpas _op_copy_rel_p_mas_dp
#define _op_copy_rel_pan_mas_dpas _op_copy_rel_pan_mas_dp
#define _op_copy_rel_pas_mas_dpas _op_copy_rel_pas_mas_dp

static void
init_copy_rel_pixel_mask_span_funcs_c(void)
{
   op_copy_rel_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_p_mas_dp;
   op_copy_rel_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_pan_mas_dp;
   op_copy_rel_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_pas_mas_dp;

   op_copy_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_p_mas_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_pan_mas_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_pas_mas_dpan;

   op_copy_rel_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_p_mas_dpas;
   op_copy_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_pan_mas_dpas;
   op_copy_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_pas_mas_dpas;
}
#endif

#ifdef BUILD_C
static void 
_op_copy_rel_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 da = 256 - (((256 - (s >> 24)) * m) >> 8);
	*d = MUL_A_256(da, *d) + INTERP_RGB_256(m + 1, s, *d);
}

static void 
_op_copy_rel_pt_pan_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d & 0xff000000) + INTERP_RGB_256(m + 1, s, *d);
}

#define _op_copy_rel_pt_pas_mas_dp _op_copy_rel_pt_p_mas_dp

#define _op_copy_rel_pt_p_mas_dpan _op_copy_pt_p_mas_dpan
#define _op_copy_rel_pt_pan_mas_dpan _op_copy_pt_pan_mas_dpan
#define _op_copy_rel_pt_pas_mas_dpan _op_copy_pt_pas_mas_dpan

#define _op_copy_rel_pt_p_mas_dpas _op_copy_rel_pt_p_mas_dp
#define _op_copy_rel_pt_pan_mas_dpas _op_copy_rel_pt_pan_mas_dp
#define _op_copy_rel_pt_pas_mas_dpas _op_copy_rel_pt_pas_mas_dp

static void
init_copy_rel_pixel_mask_pt_funcs_c(void)
{
   op_copy_rel_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_pt_p_mas_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_pt_pan_mas_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_copy_rel_pt_pas_mas_dp;

   op_copy_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_p_mas_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_pan_mas_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_pas_mas_dpan;

   op_copy_rel_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_p_mas_dpas;
   op_copy_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_pan_mas_dpas;
   op_copy_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_copy_rel_pt_pas_mas_dpas;
}
#endif
