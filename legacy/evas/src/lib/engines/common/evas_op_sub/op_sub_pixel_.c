
/* sub pixel --> dst */

#ifdef BUILD_C
static void
_op_sub_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 saa = 1 + _evas_pow_lut[((*s >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (*s >> 24)];
	MUL2_SUB_256(saa, daa, *s, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	     case 0:
		break;
	     case 0xff000000:
		MUL_SUB_REV_SYM(*d >> 24, *d, *s, *d)
		break;
	     default:
	       {
		DATA32 saa = 1 + _evas_pow_lut[((*s >> 16) & 0xff00) + (*d >> 24)];
		DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (*s >> 24)];
		MUL2_SUB_256(saa, daa, *s, *d, *d)
	       }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_sub_pan_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	MUL_SUB_REV_SYM(*d >> 24, *d, *s, *d)
	s++;  d++;
     }
}

static void
_op_sub_p_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	MUL_SUB_SYM(*s >> 24, *s, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_pas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	     case 0:
		break;
	     case 0xff000000:
		SUB(*s, *d, *d)
		break;
	     default:
		MUL_SUB_SYM(*s >> 24, *s, *d, *d)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_sub_pan_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	SUB(*s, *d, *d)
	s++;  d++;
     }
}

#define _op_sub_p_dpas _op_sub_p_dp
#define _op_sub_pas_dpas _op_sub_pas_dp
#define _op_sub_pan_dpas _op_sub_pan_dp

static void
init_sub_pixel_span_funcs_c(void)
{
   op_sub_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_sub_p_dp;
   op_sub_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_sub_pas_dp;
   op_sub_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_sub_pan_dp;

   op_sub_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_p_dpan;
   op_sub_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_pas_dpan;
   op_sub_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_pan_dpan;

   op_sub_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_p_dpas;
   op_sub_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_pas_dpas;
   op_sub_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_pan_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_sub_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 saa = 1 + _evas_pow_lut[((s >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (s >> 24)];
	MUL2_SUB_256(saa, daa, s, *d, *d)
}

static void
_op_sub_pt_pan_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_SUB_REV_SYM(*d >> 24, *d, s, *d)
}

static void
_op_sub_pt_p_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_SUB_SYM(s >> 24, s, *d, *d)
}

static void
_op_sub_pt_pan_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	SUB(s, *d, *d)
}

#define _op_sub_pt_pas_dp _op_sub_pt_p_dp
#define _op_sub_pt_pas_dpan _op_sub_pt_p_dpan

#define _op_sub_pt_p_dpas _op_sub_pt_p_dp
#define _op_sub_pt_pas_dpas _op_sub_pt_pas_dp
#define _op_sub_pt_pan_dpas _op_sub_pt_pan_dp

static void
init_sub_pixel_pt_funcs_c(void)
{
   op_sub_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_sub_pt_p_dp;
   op_sub_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_sub_pt_pas_dp;
   op_sub_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_sub_pt_pan_dp;

   op_sub_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_pt_p_dpan;
   op_sub_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_pt_pas_dpan;
   op_sub_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_pt_pan_dpan;

   op_sub_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_pt_p_dpas;
   op_sub_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_pt_pas_dpas;
   op_sub_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_pt_pan_dpas;
}
#endif

/*-----*/

/* sub_rel pixel --> dst */

#ifdef BUILD_C
static void
_op_sub_rel_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	MUL_SUB_REL_SYM(*s >> 24, *s, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_rel_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	     case 0:
		break;
	     case 0xff000000:
		SUB_REL(*s, *d, *d)
		break;
	     default:
		MUL_SUB_REL_SYM(*s >> 24, *s, *d, *d)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_sub_rel_pan_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	SUB_REL(*s, *d, *d)
	s++;  d++;
     }
}

#define _op_sub_rel_p_dpan _op_sub_p_dpan
#define _op_sub_rel_pas_dpan _op_sub_pas_dpan
#define _op_sub_rel_pan_dpan _op_sub_pan_dpan

#define _op_sub_rel_p_dpas _op_sub_rel_p_dp
#define _op_sub_rel_pas_dpas _op_sub_rel_pas_dp
#define _op_sub_rel_pan_dpas _op_sub_rel_pan_dp

static void
init_sub_rel_pixel_span_funcs_c(void)
{
   op_sub_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_p_dp;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_pas_dp;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_pan_dp;

   op_sub_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_p_dpan;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_pas_dpan;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_pan_dpan;

   op_sub_rel_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_p_dpas;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_pas_dpas;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_pan_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_sub_rel_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_SUB_REL_SYM(s >> 24, s, *d, *d)
}

static void
_op_sub_rel_pt_pan_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	SUB_REL(s, *d, *d)
}

#define _op_sub_rel_pt_pas_dp _op_sub_pt_p_dp

#define _op_sub_rel_pt_p_dpan _op_sub_pt_p_dpan
#define _op_sub_rel_pt_pas_dpan _op_sub_pt_pas_dpan
#define _op_sub_rel_pt_pan_dpan _op_sub_pt_pan_dpan

#define _op_sub_rel_pt_p_dpas _op_sub_rel_pt_p_dp
#define _op_sub_rel_pt_pas_dpas _op_sub_rel_pt_pas_dp
#define _op_sub_rel_pt_pan_dpas _op_sub_rel_pt_pan_dp

static void
init_sub_rel_pixel_pt_funcs_c(void)
{
   op_sub_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_pt_p_dp;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_pt_pas_dp;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_sub_rel_pt_pan_dp;

   op_sub_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_pt_p_dpan;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_pt_pas_dpan;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_sub_rel_pt_pan_dpan;

   op_sub_rel_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_pt_p_dpas;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_pt_pas_dpas;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_sub_rel_pt_pan_dpas;
}
#endif
