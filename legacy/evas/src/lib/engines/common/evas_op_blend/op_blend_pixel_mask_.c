
/* blend pixel x mask --> dst */

#ifdef BUILD_C
static void
_op_blend_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
	      {
		DATA32  da = (*s >> 16) & 0xff00;
		a = 1 + (da >> 8);
		da = 1 + _evas_pow_lut[da + (*d >> 24)];
		*d = BLEND_ARGB_256(a, da, *s, *d);
	      }
		break;
	    default:
	      {
		DATA32 da = ((a * (*s >> 24)) + 255) & 0xff00;
		a = 1 + (da >> 8);
		da = 1 + _evas_pow_lut[da + (*d >> 24)];
		*d = BLEND_ARGB_256(a, da, *s, *d);
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pas_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = (*s >> 24);
	switch(*m & a)
	  {
	    case 0:
		break;
	    case 255:
		*d = *s;
		break;
	    default:
	      {
		DATA32 da = ((*m * a) + 255) & 0xff00;
		a = 1 + (da >> 8);
		da = 1 + _evas_pow_lut[da + (*d >> 24)];
		*d = BLEND_ARGB_256(a, da, *s, *d);
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pan_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
		DATA32  da = 1 + _evas_pow_lut[(a << 8) + (*d >> 24)];
		a++;
		*d = BLEND_ARGB_256(a, da, *s, *d);
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_p_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		a = 1 + (*s >> 24);
		*d = BLEND_RGB_256(a, *s, *d);
		break;
	    default:
		a = 1 + (((a * (*s >> 24)) + 255) >> 8);
		*d = BLEND_RGB_256(a, *s, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pas_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = (*s >> 24);
	switch(*m & a)
	  {
	    case 0:
		break;
	    case 255:
		*d = *s;
		break;
	    default:
		a = 1 + (((*m * a) + 255) >> 8);
		*d = BLEND_RGB_256(a, *s, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pan_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
		*d = BLEND_RGB_256(a, *s, *d);
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_blend_p_mas_dpas _op_blend_p_mas_dp
#define _op_blend_pas_mas_dpas _op_blend_pas_mas_dp
#define _op_blend_pan_mas_dpas _op_blend_pan_mas_dp

static void
init_blend_pixel_mask_span_funcs_c(void)
{
   op_blend_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_blend_p_mas_dp;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_blend_pas_mas_dp;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_blend_pan_mas_dp;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_p_mas_dpan;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_pas_mas_dpan;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_pan_mas_dpan;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_p_mas_dpas;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_pas_mas_dpas;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_pan_mas_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 da = ((m * (s >> 24)) + 255) & 0xff00;
	c = 1 + (da >> 8);
	da = 1 + _evas_pow_lut[da + (*d >> 24)];
	*d = BLEND_ARGB_256(c, da, s, *d);
}

static void
_op_blend_pt_pan_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + _evas_pow_lut[(m << 8) + (*d >> 24)];
	*d = BLEND_ARGB_256(1 + m, c, s, *d);
}

static void
_op_blend_pt_p_mas_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + (((m * (s >> 24)) + 255) >> 8);
	*d = BLEND_RGB_256(c, s, *d);
}

static void
_op_blend_pt_pan_mas_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = BLEND_RGB_256(1 + m, s, *d);
}

#define _op_blend_pt_pas_mas_dp _op_blend_pt_p_mas_dp

#define _op_blend_pt_pas_mas_dpan _op_blend_pt_p_mas_dpan

#define _op_blend_pt_p_mas_dpas _op_blend_pt_p_mas_dp
#define _op_blend_pt_pas_mas_dpas _op_blend_pt_pas_mas_dp
#define _op_blend_pt_pan_mas_dpas _op_blend_pt_pan_mas_dp

static void
init_blend_pixel_mask_pt_funcs_c(void)
{
   op_blend_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_blend_pt_p_mas_dp;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_blend_pt_pas_mas_dp;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_blend_pt_pan_mas_dp;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_pt_p_mas_dpan;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_pt_pas_mas_dpan;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_pt_pan_mas_dpan;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_pt_p_mas_dpas;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_pt_pas_mas_dpas;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_pt_pan_mas_dpas;
}
#endif

/*-----*/

/* blend_rel pixel x mask -> dst */

#ifdef BUILD_C
#define _op_blend_rel_p_mas_dp _op_blend_p_mas_dpan
#define _op_blend_rel_pas_mas_dp _op_blend_pas_mas_dpan
#define _op_blend_rel_pan_mas_dp _op_blend_pan_mas_dpan

#define _op_blend_rel_p_mas_dpan _op_blend_p_mas_dpan
#define _op_blend_rel_pas_mas_dpan _op_blend_pas_mas_dpan
#define _op_blend_rel_pan_mas_dpan _op_blend_pan_mas_dpan

#define _op_blend_rel_p_mas_dpas _op_blend_rel_p_mas_dp
#define _op_blend_rel_pas_mas_dpas _op_blend_rel_pas_mas_dp
#define _op_blend_rel_pan_mas_dpas _op_blend_rel_pan_mas_dp

static void
init_blend_rel_pixel_mask_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_p_mas_dp;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_pas_mas_dp;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_pan_mas_dp;

   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_p_mas_dpan;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_pas_mas_dpan;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_pan_mas_dpan;

   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_p_mas_dpas;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_pas_mas_dpas;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_pan_mas_dpas;
}
#endif

#ifdef BUILD_C
#define _op_blend_rel_pt_p_mas_dp _op_blend_pt_p_mas_dpan
#define _op_blend_rel_pt_pas_mas_dp _op_blend_pt_pas_mas_dpan
#define _op_blend_rel_pt_pan_mas_dp _op_blend_pt_pan_mas_dpan

#define _op_blend_rel_pt_p_mas_dpan _op_blend_pt_p_mas_dpan
#define _op_blend_rel_pt_pas_mas_dpan _op_blend_pt_pas_mas_dpan
#define _op_blend_rel_pt_pan_mas_dpan _op_blend_pt_pan_mas_dpan

#define _op_blend_rel_pt_p_mas_dpas _op_blend_rel_pt_p_mas_dp
#define _op_blend_rel_pt_pas_mas_dpas _op_blend_rel_pt_pas_mas_dp
#define _op_blend_rel_pt_pan_mas_dpas _op_blend_rel_pt_pan_mas_dp

static void
init_blend_rel_pixel_mask_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_pt_p_mas_dp;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_pt_pas_mas_dp;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_blend_rel_pt_pan_mas_dp;

   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_p_mas_dpan;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_pas_mas_dpan;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_pan_mas_dpan;

   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_pt_p_mas_dpas;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_pt_pas_mas_dpas;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_blend_rel_pt_pan_mas_dpas;
}
#endif
