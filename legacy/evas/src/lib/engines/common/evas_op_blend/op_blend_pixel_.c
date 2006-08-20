
/* blend pixel --> dst */

#ifdef BUILD_C
static void
_op_blend_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32  a = *s >> 24;
	DATA32  da = 1 + _evas_pow_lut[(a << 8) + (*d >> 24)];
	a++;
	*d++ = BLEND_ARGB_256(a, da, *s, *d);
	s++;
     }
}

static void
_op_blend_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	     case 0:
		break;
	     case 0xff000000:
		*d = *s;
		break;
	     default:
	      {
		DATA32  da = *d >> 24;
		a >>= 24;
		da = 1 + _evas_pow_lut[(a << 8) + da];
		a++;
		*d = BLEND_ARGB_256(a, da, *s, *d);
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_p_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32  a = 1 + (*s >> 24);
	*d++ = BLEND_RGB_256(a, *s, *d);
	s++;
     }
}

static void
_op_blend_pas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	     case 0:
		break;
	     case 0xff000000:
		*d = *s;
		break;
	     default:
		a = 1 + (a >> 24);
		*d = BLEND_RGB_256(a, *s, *d);
		break;
	  }
	s++;  d++;
     }
}

#define _op_blend_pan_dp NULL

#define _op_blend_pan_dpan _op_blend_pan_dp

#define _op_blend_p_dpas _op_blend_p_dp
#define _op_blend_pas_dpas _op_blend_pas_dp
#define _op_blend_pan_dpas _op_blend_pan_dp

static void
init_blend_pixel_span_funcs_c(void)
{
   op_blend_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_p_dp;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_pas_dp;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_pan_dp;

   op_blend_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_p_dpan;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pas_dpan;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pan_dpan;

   op_blend_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_p_dpas;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_pas_dpas;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_pan_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32  da = 1 + _evas_pow_lut[((s >> 16) & 0xff00) + (*d >> 24)];
	*d = BLEND_ARGB_256(1 + (s >> 24), da, s, *d);
}

static void
_op_blend_pt_p_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = BLEND_RGB_256(1 + (s >> 24), s, *d);
}

#define _op_blend_pt_pas_dp _op_blend_pt_p_dp
#define _op_blend_pt_pan_dp NULL

#define _op_blend_pt_pan_dpan _op_blend_pt_pan_dp
#define _op_blend_pt_pas_dpan _op_blend_pt_p_dpan

#define _op_blend_pt_p_dpas _op_blend_pt_p_dp
#define _op_blend_pt_pas_dpas _op_blend_pt_pas_dp
#define _op_blend_pt_pan_dpas _op_blend_pt_pan_dp

static void
init_blend_pixel_pt_funcs_c(void)
{
   op_blend_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_pt_p_dp;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_pt_pas_dp;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_pt_pan_dp;

   op_blend_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pt_p_dpan;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pt_pas_dpan;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pt_pan_dpan;

   op_blend_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_pt_p_dpas;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_pt_pas_dpas;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_pt_pan_dpas;
}
#endif

/*-----*/

/* blend_rel pixel -> dst */

#ifdef BUILD_C
#define _op_blend_rel_p_dp _op_blend_p_dpan 
#define _op_blend_rel_pan_dp _op_blend_pan_dpan 
#define _op_blend_rel_pas_dp _op_blend_pas_dpan 

#define _op_blend_rel_p_dpan _op_blend_p_dpan
#define _op_blend_rel_pan_dpan _op_blend_pan_dpan
#define _op_blend_rel_pas_dpan _op_blend_pas_dpan

#define _op_blend_rel_p_dpas _op_blend_rel_p_dp
#define _op_blend_rel_pan_dpas _op_blend_rel_pan_dp
#define _op_blend_rel_pas_dpas _op_blend_rel_pas_dp

static void
init_blend_rel_pixel_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_p_dp;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pas_dp;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pan_dp;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_p_dpan;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pas_dpan;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pan_dpan;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_rel_p_dpas;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_rel_pas_dpas;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_rel_pan_dpas;
}
#endif

#ifdef BUILD_C
#define _op_blend_rel_pt_p_dp _op_blend_pt_p_dpan 
#define _op_blend_rel_pt_pan_dp _op_blend_pt_pan_dpan 
#define _op_blend_rel_pt_pas_dp _op_blend_pt_pas_dpan 

#define _op_blend_rel_pt_p_dpan _op_blend_pt_p_dpan
#define _op_blend_rel_pt_pan_dpan _op_blend_pt_pan_dpan
#define _op_blend_rel_pt_pas_dpan _op_blend_pt_pas_dpan

#define _op_blend_rel_pt_p_dpas _op_blend_rel_pt_p_dp
#define _op_blend_rel_pt_pan_dpas _op_blend_rel_pt_pan_dp
#define _op_blend_rel_pt_pas_dpas _op_blend_rel_pt_pas_dp

static void
init_blend_rel_pixel_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pt_p_dp;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pt_pas_dp;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pt_pan_dp;

   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_p_dpan;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_pas_dpan;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_pan_dpan;

   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_rel_pt_p_dpas;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_rel_pt_pas_dpas;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_blend_rel_pt_pan_dpas;
}
#endif
