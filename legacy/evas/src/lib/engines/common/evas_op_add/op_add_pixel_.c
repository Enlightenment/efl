
/* add pixel --> dst */

#ifdef BUILD_C
static void
_op_add_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 saa = 1 + _evas_pow_lut[((*s >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (*s >> 24)];
	MUL2_ADD_256(saa, daa, *s, *d, *d)
	s++;  d++;
     }
}

static void
_op_add_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	     case 0:
		break;
	     case 0xff000000:
		MUL_ADD_SYM(*d >> 24, *d, *s, *d)
		break;
	     default:
	       {
		DATA32 saa = 1 + _evas_pow_lut[((*s >> 16) & 0xff00) + (*d >> 24)];
		DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (*s >> 24)];
		MUL2_ADD_256(saa, daa, *s, *d, *d)
	       }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_add_pan_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	MUL_ADD_SYM(*d >> 24, *d, *s, *d)
	s++;  d++;
     }
}

static void
_op_add_p_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	MUL_ADD_SYM(*s >> 24, *s, *d, *d)
	s++;  d++;
     }
}

static void
_op_add_pas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	     case 0:
		break;
	     case 0xff000000:
		ADD(*s, *d, *d)
		break;
	     default:
		MUL_ADD_SYM(*s >> 24, *s, *d, *d)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_add_pan_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	ADD(*s, *d, *d)
	s++;  d++;
     }
}

#define _op_add_p_dpas _op_add_p_dp
#define _op_add_pas_dpas _op_add_pas_dp
#define _op_add_pan_dpas _op_add_pan_dp

static void
init_add_pixel_span_funcs_c(void)
{
   op_add_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_add_p_dp;
   op_add_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_add_pas_dp;
   op_add_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_add_pan_dp;

   op_add_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_add_p_dpan;
   op_add_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_add_pas_dpan;
   op_add_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_add_pan_dpan;

   op_add_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_add_p_dpas;
   op_add_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_add_pas_dpas;
   op_add_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_add_pan_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_add_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 saa = 1 + _evas_pow_lut[((s >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (s >> 24)];
	MUL2_ADD_256(saa, daa, s, *d, *d)
}

static void
_op_add_pt_pan_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_ADD_SYM(*d >> 24, *d, s, *d)
}

static void
_op_add_pt_p_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_ADD_SYM(s >> 24, s, *d, *d)
}

static void
_op_add_pt_pan_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	ADD(s, *d, *d)
}

#define _op_add_pt_pas_dp _op_add_pt_p_dp
#define _op_add_pt_pas_dpan _op_add_pt_p_dpan

#define _op_add_pt_p_dpas _op_add_pt_p_dp
#define _op_add_pt_pas_dpas _op_add_pt_pas_dp
#define _op_add_pt_pan_dpas _op_add_pt_pan_dp

static void
init_add_pixel_pt_funcs_c(void)
{
   op_add_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_add_pt_p_dp;
   op_add_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_add_pt_pas_dp;
   op_add_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_add_pt_pan_dp;

   op_add_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_add_pt_p_dpan;
   op_add_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_add_pt_pas_dpan;
   op_add_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_add_pt_pan_dpan;

   op_add_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_add_pt_p_dpas;
   op_add_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_add_pt_pas_dpas;
   op_add_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_add_pt_pan_dpas;
}
#endif

/*-----*/

/* add_rel pixel --> dst */

#ifdef BUILD_C
static void
_op_add_rel_p_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	MUL_ADD_REL_SYM(*s >> 24, *s, *d, *d)
	s++;  d++;
     }
}

static void
_op_add_rel_pas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	     case 0:
		break;
	     case 0xff000000:
		ADD_REL(*s, *d, *d)
		break;
	     default:
		MUL_ADD_REL_SYM(*s >> 24, *s, *d, *d)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_add_rel_pan_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	ADD_REL(*s, *d, *d)
	s++;  d++;
     }
}

#define _op_add_rel_p_dpan _op_add_p_dpan
#define _op_add_rel_pas_dpan _op_add_pas_dpan
#define _op_add_rel_pan_dpan _op_add_pan_dpan

#define _op_add_rel_p_dpas _op_add_rel_p_dp
#define _op_add_rel_pas_dpas _op_add_rel_pas_dp
#define _op_add_rel_pan_dpas _op_add_rel_pan_dp

static void
init_add_rel_pixel_span_funcs_c(void)
{
   op_add_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_add_rel_p_dp;
   op_add_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_add_rel_pas_dp;
   op_add_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_add_rel_pan_dp;

   op_add_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_add_rel_p_dpan;
   op_add_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_add_rel_pas_dpan;
   op_add_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_add_rel_pan_dpan;

   op_add_rel_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_add_rel_p_dpas;
   op_add_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_add_rel_pas_dpas;
   op_add_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_add_rel_pan_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_add_rel_pt_p_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_ADD_REL_SYM(s >> 24, s, *d, *d)
}

static void
_op_add_rel_pt_pan_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	ADD_REL(s, *d, *d)
}

#define _op_add_rel_pt_pas_dp _op_add_pt_p_dp

#define _op_add_rel_pt_p_dpan _op_add_pt_p_dpan
#define _op_add_rel_pt_pas_dpan _op_add_pt_pas_dpan
#define _op_add_rel_pt_pan_dpan _op_add_pt_pan_dpan

#define _op_add_rel_pt_p_dpas _op_add_rel_pt_p_dp
#define _op_add_rel_pt_pas_dpas _op_add_rel_pt_pas_dp
#define _op_add_rel_pt_pan_dpas _op_add_rel_pt_pan_dp

static void
init_add_rel_pixel_pt_funcs_c(void)
{
   op_add_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_add_rel_pt_p_dp;
   op_add_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_add_rel_pt_pas_dp;
   op_add_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_add_rel_pt_pan_dp;

   op_add_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_add_rel_pt_p_dpan;
   op_add_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_add_rel_pt_pas_dpan;
   op_add_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_add_rel_pt_pan_dpan;

   op_add_rel_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_C] = _op_add_rel_pt_p_dpas;
   op_add_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_C] = _op_add_rel_pt_pas_dpas;
   op_add_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_C] = _op_add_rel_pt_pan_dpas;
}
#endif
