
/* sub pixel x color --> dst */

#ifdef BUILD_C
static void
_op_sub_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = MUL4_SYM(c, *s);
	DATA32 saa = 1 + _evas_pow_lut[((sc >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (sc >> 24)];
	MUL2_SUB_256(saa, daa, sc, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_pas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		a = 0xff000000 + MUL3_SYM(c, *s);
		MUL_SUB_REV_SYM(*d >> 24, *d, a, *d)
		break;
	    default:
	      {
		DATA32 sc = a + MUL3_SYM(c, *s);
		DATA32 saa = 1 + _evas_pow_lut[((a >> 16) & 0xff00) | (*d >> 24)];
		DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (*s >> 24)];
		MUL2_SUB_256(saa, daa, sc, *d, *d)
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_sub_pan_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = 0xff000000 + MUL3_SYM(c, *s);
	MUL_SUB_REV_SYM(*d >> 24, *d, sc, *d)
	s++;  d++;
     }
}

static void
_op_sub_p_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 daa = ((*s >> 24) * ca) >> 8;
	DATA32 saa = 1 + _evas_pow_lut[(daa << 8) + (*d >> 24)];
	daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + daa];
	MUL2_SUB_256(saa, daa, *s, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_p_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = MUL4_SYM(c, *s);
	MUL_SUB_SYM(sc >> 24, sc, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_pas_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		a = 0xff000000 + MUL3_SYM(c, *s);
		SUB(a, *d, *d)
		break;
	    default:
		a = a + MUL3_SYM(c, *s);
		MUL_SUB_SYM(a >> 24, a, *d, *d)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_sub_pan_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = 0xff000000 + MUL3_SYM(c, *s);
	SUB(sc, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_p_caa_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 sc = MUL_A_256(ca, *s) + (*s & 0x00ffffff);
	MUL_SUB_SYM(sc >> 24, sc, *d, *d)
	s++;  d++;
     }
}


#define _op_sub_pas_c_dp _op_sub_p_c_dp
#define _op_sub_pan_c_dp _op_sub_p_c_dp
#define _op_sub_p_can_dp _op_sub_p_c_dp
#define _op_sub_pas_caa_dp _op_sub_p_caa_dp
#define _op_sub_pan_caa_dp _op_sub_p_caa_dp

#define _op_sub_pas_c_dpan _op_sub_p_c_dpan
#define _op_sub_pan_c_dpan _op_sub_p_c_dpan
#define _op_sub_p_can_dpan _op_sub_p_c_dpan
#define _op_sub_pas_can_dpan _op_sub_p_can_dpan
#define _op_sub_pas_caa_dpan _op_sub_p_caa_dpan
#define _op_sub_pan_caa_dpan _op_sub_p_caa_dpan

#define _op_sub_p_c_dpas _op_sub_p_c_dp
#define _op_sub_pas_c_dpas _op_sub_pas_c_dp
#define _op_sub_pan_c_dpas _op_sub_pan_c_dp
#define _op_sub_p_can_dpas _op_sub_p_can_dp
#define _op_sub_pas_can_dpas _op_sub_pas_can_dp
#define _op_sub_pan_can_dpas _op_sub_pan_can_dp
#define _op_sub_p_caa_dpas _op_sub_p_caa_dp
#define _op_sub_pas_caa_dpas _op_sub_pas_caa_dp
#define _op_sub_pan_caa_dpas _op_sub_pan_caa_dp

static void
init_sub_pixel_color_span_funcs_c(void)
{
   op_sub_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_sub_p_c_dp;
   op_sub_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_sub_pas_c_dp;
   op_sub_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_sub_pan_c_dp;
   op_sub_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_sub_p_can_dp;
   op_sub_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_sub_pas_can_dp;
   op_sub_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_sub_pan_can_dp;
   op_sub_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_sub_p_caa_dp;
   op_sub_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_sub_pas_caa_dp;
   op_sub_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_sub_pan_caa_dp;

   op_sub_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_sub_p_c_dpan;
   op_sub_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_sub_pas_c_dpan;
   op_sub_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_sub_pan_c_dpan;
   op_sub_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_p_can_dpan;
   op_sub_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_pas_can_dpan;
   op_sub_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_pan_can_dpan;
   op_sub_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_p_caa_dpan;
   op_sub_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_pas_caa_dpan;
   op_sub_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_pan_caa_dpan;

   op_sub_span_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_sub_p_c_dpas;
   op_sub_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_sub_pas_c_dpas;
   op_sub_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_sub_pan_c_dpas;
   op_sub_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_p_can_dpas;
   op_sub_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_pas_can_dpas;
   op_sub_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_pan_can_dpas;
   op_sub_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_p_caa_dpas;
   op_sub_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_pas_caa_dpas;
   op_sub_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_pan_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_sub_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 sc = MUL4_SYM(c, s);
	DATA32 saa = 1 + _evas_pow_lut[((sc >> 16) & 0xff00) + (*d >> 24)];
	DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (sc >> 24)];
	MUL2_SUB_256(saa, daa, sc, *d, *d)
}

static void
_op_sub_pt_pan_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 0xff000000 + MUL3_SYM(c, s);
	MUL_SUB_REV_SYM(*d >> 24, *d, s, *d)
}

static void
_op_sub_pt_p_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 daa = (((s >> 24) * (c >> 24)) + 255) >> 8;
	DATA32 saa = 1 + _evas_pow_lut[(daa << 8) + (*d >> 24)];
	daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + daa];
	MUL2_SUB_256(saa, daa, s, *d, *d)
}

static void
_op_sub_pt_p_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL4_SYM(c, s);
	MUL_SUB_SYM(s >> 24, s, *d, *d)
}

static void
_op_sub_pt_pan_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = 0xff000000 + MUL3_SYM(c, s);
	SUB(s, *d, *d)
}

static void
_op_sub_pt_p_caa_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL_A_SYM(c >> 24, s) + (s & 0x00ffffff);
	MUL_SUB_SYM(s >> 24, s, *d, *d)
}


#define _op_sub_pt_pas_c_dp _op_sub_pt_p_c_dp
#define _op_sub_pt_pan_c_dp _op_sub_pt_p_c_dp
#define _op_sub_pt_p_can_dp _op_sub_pt_p_c_dp
#define _op_sub_pt_pas_can_dp _op_sub_pt_p_can_dp
#define _op_sub_pt_pas_caa_dp _op_sub_pt_p_caa_dp
#define _op_sub_pt_pan_caa_dp _op_sub_pt_p_caa_dp

#define _op_sub_pt_pas_c_dpan _op_sub_pt_p_c_dpan
#define _op_sub_pt_pan_c_dpan _op_sub_pt_p_c_dpan
#define _op_sub_pt_p_can_dpan _op_sub_pt_p_c_dpan
#define _op_sub_pt_pas_can_dpan _op_sub_pt_p_can_dpan
#define _op_sub_pt_pas_caa_dpan _op_sub_pt_p_caa_dpan
#define _op_sub_pt_pan_caa_dpan _op_sub_pt_p_caa_dpan

#define _op_sub_pt_p_c_dpas _op_sub_pt_p_c_dp
#define _op_sub_pt_pas_c_dpas _op_sub_pt_pas_c_dp
#define _op_sub_pt_pan_c_dpas _op_sub_pt_pan_c_dp
#define _op_sub_pt_p_can_dpas _op_sub_pt_p_can_dp
#define _op_sub_pt_pas_can_dpas _op_sub_pt_pas_can_dp
#define _op_sub_pt_pan_can_dpas _op_sub_pt_pan_can_dp
#define _op_sub_pt_p_caa_dpas _op_sub_pt_p_caa_dp
#define _op_sub_pt_pas_caa_dpas _op_sub_pt_pas_caa_dp
#define _op_sub_pt_pan_caa_dpas _op_sub_pt_pan_caa_dp

static void
init_sub_pixel_color_pt_funcs_c(void)
{
   op_sub_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_sub_pt_p_c_dp;
   op_sub_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_sub_pt_pas_c_dp;
   op_sub_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_sub_pt_pan_c_dp;
   op_sub_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_sub_pt_p_can_dp;
   op_sub_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_sub_pt_pas_can_dp;
   op_sub_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_sub_pt_pan_can_dp;
   op_sub_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_sub_pt_p_caa_dp;
   op_sub_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_sub_pt_pas_caa_dp;
   op_sub_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_sub_pt_pan_caa_dp;

   op_sub_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_sub_pt_p_c_dpan;
   op_sub_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_sub_pt_pas_c_dpan;
   op_sub_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_sub_pt_pan_c_dpan;
   op_sub_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_pt_p_can_dpan;
   op_sub_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_pt_pas_can_dpan;
   op_sub_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_pt_pan_can_dpan;
   op_sub_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_pt_p_caa_dpan;
   op_sub_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_pt_pas_caa_dpan;
   op_sub_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_pt_pan_caa_dpan;

   op_sub_pt_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_sub_pt_p_c_dpas;
   op_sub_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_sub_pt_pas_c_dpas;
   op_sub_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_sub_pt_pan_c_dpas;
   op_sub_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_pt_p_can_dpas;
   op_sub_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_pt_pas_can_dpas;
   op_sub_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_pt_pan_can_dpas;
   op_sub_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_pt_p_caa_dpas;
   op_sub_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_pt_pas_caa_dpas;
   op_sub_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_pt_pan_caa_dpas;
}
#endif

/*-----*/

/* sub_rel pixel x color --> dst */

#ifdef BUILD_C
static void
_op_sub_rel_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = MUL4_SYM(c, *s);
	MUL_SUB_REL_SYM(sc >> 24, sc, *d, *d)
	s++;  d++;
     }
}

static void
_op_sub_rel_p_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 sc = MUL_A_256(c, *s) + (*s & 0x00ffffff);
	MUL_SUB_REL_SYM(sc >> 24, sc, *d, *d)
	s++;  d++;
     }
}

#define _op_sub_rel_pas_c_dp _op_sub_rel_p_c_dp
#define _op_sub_rel_pan_c_dp _op_sub_rel_p_c_dp
#define _op_sub_rel_p_can_dp _op_sub_rel_p_c_dp
#define _op_sub_rel_pas_can_dp _op_sub_rel_p_can_dp
#define _op_sub_rel_pan_can_dp _op_sub_rel_p_can_dp
#define _op_sub_rel_pas_caa_dp _op_sub_rel_p_caa_dp
#define _op_sub_rel_pan_caa_dp _op_sub_rel_p_caa_dp

#define _op_sub_rel_p_c_dpan _op_sub_p_c_dpan
#define _op_sub_rel_pas_c_dpan _op_sub_pas_c_dpan
#define _op_sub_rel_pan_c_dpan _op_sub_pan_c_dpan
#define _op_sub_rel_p_can_dpan _op_sub_p_can_dpan
#define _op_sub_rel_pas_can_dpan _op_sub_pas_can_dpan
#define _op_sub_rel_pan_can_dpan _op_sub_pan_can_dpan
#define _op_sub_rel_p_caa_dpan _op_sub_p_caa_dpan
#define _op_sub_rel_pas_caa_dpan _op_sub_pas_caa_dpan
#define _op_sub_rel_pan_caa_dpan _op_sub_pan_caa_dpan

#define _op_sub_rel_p_c_dpas _op_sub_rel_p_c_dp
#define _op_sub_rel_pas_c_dpas _op_sub_rel_pas_c_dp
#define _op_sub_rel_pan_c_dpas _op_sub_rel_pan_c_dp
#define _op_sub_rel_p_can_dpas _op_sub_rel_p_can_dp
#define _op_sub_rel_pas_can_dpas _op_sub_rel_pas_can_dp
#define _op_sub_rel_pan_can_dpas _op_sub_rel_pan_can_dp
#define _op_sub_rel_p_caa_dpas _op_sub_rel_p_caa_dp
#define _op_sub_rel_pas_caa_dpas _op_sub_rel_pas_caa_dp
#define _op_sub_rel_pan_caa_dpas _op_sub_rel_pan_caa_dp

static void
init_sub_rel_pixel_color_span_funcs_c(void)
{
   op_sub_rel_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_sub_rel_p_c_dp;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_sub_rel_pas_c_dp;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_sub_rel_pan_c_dp;
   op_sub_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_p_can_dp;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_pas_can_dp;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_pan_can_dp;
   op_sub_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_p_caa_dp;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_pas_caa_dp;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_pan_caa_dp;

   op_sub_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_p_c_dpan;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_pas_c_dpan;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_pan_c_dpan;
   op_sub_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_p_can_dpan;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_pas_can_dpan;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_pan_can_dpan;
   op_sub_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_p_caa_dpan;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_pas_caa_dpan;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_pan_caa_dpan;

   op_sub_rel_span_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_p_c_dpas;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_pas_c_dpas;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_pan_c_dpas;
   op_sub_rel_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_p_can_dpas;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_pas_can_dpas;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_pan_can_dpas;
   op_sub_rel_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_p_caa_dpas;
   op_sub_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_pas_caa_dpas;
   op_sub_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_pan_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_sub_rel_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL4_SYM(c, s);
	MUL_SUB_REL_SYM(s >> 24, s, *d, *d)
}

static void
_op_sub_rel_pt_p_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL_A_SYM(c >> 24, s) + (s & 0x00ffffff);
	MUL_SUB_REL_SYM(s >> 24, s, *d, *d)
}

#define _op_sub_rel_pt_pas_c_dp _op_sub_rel_pt_p_c_dp
#define _op_sub_rel_pt_pan_c_dp _op_sub_rel_pt_p_c_dp
#define _op_sub_rel_pt_p_can_dp _op_sub_rel_pt_p_c_dp
#define _op_sub_rel_pt_pas_can_dp _op_sub_rel_pt_p_can_dp
#define _op_sub_rel_pt_pan_can_dp _op_sub_rel_pt_p_can_dp
#define _op_sub_rel_pt_pas_caa_dp _op_sub_rel_pt_p_caa_dp
#define _op_sub_rel_pt_pan_caa_dp _op_sub_rel_pt_p_caa_dp

#define _op_sub_rel_pt_p_c_dpan _op_sub_pt_p_c_dpan
#define _op_sub_rel_pt_pas_c_dpan _op_sub_pt_pas_c_dpan
#define _op_sub_rel_pt_pan_c_dpan _op_sub_pt_pan_c_dpan
#define _op_sub_rel_pt_p_can_dpan _op_sub_pt_p_can_dpan
#define _op_sub_rel_pt_pas_can_dpan _op_sub_pt_pas_can_dpan
#define _op_sub_rel_pt_pan_can_dpan _op_sub_pt_pan_can_dpan
#define _op_sub_rel_pt_p_caa_dpan _op_sub_pt_p_caa_dpan
#define _op_sub_rel_pt_pas_caa_dpan _op_sub_pt_pas_caa_dpan
#define _op_sub_rel_pt_pan_caa_dpan _op_sub_pt_pan_caa_dpan

#define _op_sub_rel_pt_p_c_dpas _op_sub_rel_pt_p_c_dp
#define _op_sub_rel_pt_pas_c_dpas _op_sub_rel_pt_pas_c_dp
#define _op_sub_rel_pt_pan_c_dpas _op_sub_rel_pt_pan_c_dp
#define _op_sub_rel_pt_p_can_dpas _op_sub_rel_pt_p_can_dp
#define _op_sub_rel_pt_pas_can_dpas _op_sub_rel_pt_pas_can_dp
#define _op_sub_rel_pt_pan_can_dpas _op_sub_rel_pt_pan_can_dp
#define _op_sub_rel_pt_p_caa_dpas _op_sub_rel_pt_p_caa_dp
#define _op_sub_rel_pt_pas_caa_dpas _op_sub_rel_pt_pas_caa_dp
#define _op_sub_rel_pt_pan_caa_dpas _op_sub_rel_pt_pan_caa_dp

static void
init_sub_rel_pixel_color_pt_funcs_c(void)
{
   op_sub_rel_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_sub_rel_pt_p_c_dp;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_sub_rel_pt_pas_c_dp;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_sub_rel_pt_pan_c_dp;
   op_sub_rel_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_pt_p_can_dp;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_pt_pas_can_dp;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_sub_rel_pt_pan_can_dp;
   op_sub_rel_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_pt_p_caa_dp;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_pt_pas_caa_dp;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_sub_rel_pt_pan_caa_dp;

   op_sub_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_pt_p_c_dpan;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_pt_pas_c_dpan;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_sub_rel_pt_pan_c_dpan;
   op_sub_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_pt_p_can_dpan;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_pt_pas_can_dpan;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_sub_rel_pt_pan_can_dpan;
   op_sub_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_pt_p_caa_dpan;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_pt_pas_caa_dpan;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_sub_rel_pt_pan_caa_dpan;

   op_sub_rel_pt_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_pt_p_c_dpas;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_pt_pas_c_dpas;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_sub_rel_pt_pan_c_dpas;
   op_sub_rel_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_pt_p_can_dpas;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_pt_pas_can_dpas;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_sub_rel_pt_pan_can_dpas;
   op_sub_rel_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_pt_p_caa_dpas;
   op_sub_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_pt_pas_caa_dpas;
   op_sub_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_sub_rel_pt_pan_caa_dpas;
}
#endif
