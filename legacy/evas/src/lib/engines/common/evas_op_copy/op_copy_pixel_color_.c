
/* copy pixel x color --> dst */

#ifdef BUILD_C
static void
_op_copy_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = MUL4_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_copy_pas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		*d = 0;
		break;
	    case 0xff000000:
		*d = (c & 0xff000000) + MUL3_SYM(c, *s);
		break;
	    default:
		*d = MUL4_SYM(c, *s);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_copy_pan_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = (c & 0xff000000) + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_copy_p_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = (*s & 0xff000000) + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_copy_pas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;

	switch(a)
	  {
	    case 0:
		*d = 0;
		break;
	    case 0xff000000:
		*d = 0xff000000 + MUL3_SYM(c, *s);
		break;
	    default:
		*d = a + MUL3_SYM(c, *s);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_copy_pan_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = 0xff000000 + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_copy_p_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	*d = MUL_A_256(c, *s) + (*s & 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_copy_pas_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		*d = *s;
		break;
	    case 0xff000000:
		*d = c & *s;
		break;
	    default:
		*d = MUL_A_256(ca, *s) + (*s & 0x00ffffff);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_copy_pan_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = c & *s;
	s++;  d++;
     }
}

#define _op_copy_p_c_dpan _op_copy_p_c_dp
#define _op_copy_pas_c_dpan _op_copy_pas_c_dp
#define _op_copy_pan_c_dpan _op_copy_pan_c_dp
#define _op_copy_p_can_dpan _op_copy_p_can_dp
#define _op_copy_pas_can_dpan _op_copy_pas_can_dp
#define _op_copy_pan_can_dpan _op_copy_pan_can_dp
#define _op_copy_p_caa_dpan _op_copy_p_caa_dp
#define _op_copy_pas_caa_dpan _op_copy_pas_caa_dp
#define _op_copy_pan_caa_dpan _op_copy_pan_caa_dp

#define _op_copy_p_c_dpas _op_copy_p_c_dp
#define _op_copy_pas_c_dpas _op_copy_pas_c_dp
#define _op_copy_pan_c_dpas _op_copy_pan_c_dp
#define _op_copy_p_can_dpas _op_copy_p_can_dp
#define _op_copy_pas_can_dpas _op_copy_pas_can_dp
#define _op_copy_pan_can_dpas _op_copy_pan_can_dp
#define _op_copy_p_caa_dpas _op_copy_p_caa_dp
#define _op_copy_pas_caa_dpas _op_copy_pas_caa_dp
#define _op_copy_pan_caa_dpas _op_copy_pan_caa_dp

static void
init_copy_pixel_color_span_funcs_c(void)
{
   op_copy_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_p_c_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_pas_c_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_pan_c_dp;
   op_copy_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_p_can_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_pas_can_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_pan_can_dp;
   op_copy_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_p_caa_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_pas_caa_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_pan_caa_dp;

   op_copy_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_p_c_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_pas_c_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_pan_c_dpan;
   op_copy_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_p_can_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pas_can_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pan_can_dpan;
   op_copy_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_p_caa_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pas_caa_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pan_caa_dpan;

   op_copy_span_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_copy_p_c_dpas;
   op_copy_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_copy_pas_c_dpas;
   op_copy_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_copy_pan_c_dpas;
   op_copy_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_p_can_dpas;
   op_copy_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_pas_can_dpas;
   op_copy_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_pan_can_dpas;
   op_copy_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_p_caa_dpas;
   op_copy_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_pas_caa_dpas;
   op_copy_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_pan_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL4_SYM(c, s);
}

static void
_op_copy_pt_pan_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (c & 0xff000000) + MUL3_SYM(c, s);
}

static void
_op_copy_pt_p_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (s & 0xff000000) + MUL3_SYM(c, s);
}

static void
_op_copy_pt_pan_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = 0xff000000 + MUL3_SYM(c, s);
}

static void
_op_copy_pt_p_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(c >> 24, s) + (s & 0x00ffffff);
}

static void
_op_copy_pt_pan_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = c & s;
}

#define _op_copy_pt_pas_c_dp _op_copy_pt_p_c_dp
#define _op_copy_pt_pas_can_dp _op_copy_pt_p_can_dp
#define _op_copy_pt_pas_caa_dp _op_copy_pt_p_caa_dp

#define _op_copy_pt_p_c_dpan _op_copy_pt_p_c_dp
#define _op_copy_pt_pas_c_dpan _op_copy_pt_pas_c_dp
#define _op_copy_pt_pan_c_dpan _op_copy_pt_pan_c_dp
#define _op_copy_pt_p_can_dpan _op_copy_pt_p_can_dp
#define _op_copy_pt_pas_can_dpan _op_copy_pt_pas_can_dp
#define _op_copy_pt_pan_can_dpan _op_copy_pt_pan_can_dp
#define _op_copy_pt_p_caa_dpan _op_copy_pt_p_caa_dp
#define _op_copy_pt_pas_caa_dpan _op_copy_pt_pas_caa_dp
#define _op_copy_pt_pan_caa_dpan _op_copy_pt_pan_caa_dp

#define _op_copy_pt_p_c_dpas _op_copy_pt_p_c_dp
#define _op_copy_pt_pas_c_dpas _op_copy_pt_pas_c_dp
#define _op_copy_pt_pan_c_dpas _op_copy_pt_pan_c_dp
#define _op_copy_pt_p_can_dpas _op_copy_pt_p_can_dp
#define _op_copy_pt_pas_can_dpas _op_copy_pt_pas_can_dp
#define _op_copy_pt_pan_can_dpas _op_copy_pt_pan_can_dp
#define _op_copy_pt_p_caa_dpas _op_copy_pt_p_caa_dp
#define _op_copy_pt_pas_caa_dpas _op_copy_pt_pas_caa_dp
#define _op_copy_pt_pan_caa_dpas _op_copy_pt_pan_caa_dp

static void
init_copy_pixel_color_pt_funcs_c(void)
{
   op_copy_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_pt_p_c_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_pt_pas_c_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_pt_pan_c_dp;
   op_copy_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_pt_p_can_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_pt_pas_can_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_pt_pan_can_dp;
   op_copy_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_pt_p_caa_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_pt_pas_caa_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_pt_pan_caa_dp;

   op_copy_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_pt_p_c_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_pt_pas_c_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_pt_pan_c_dpan;
   op_copy_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pt_p_can_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pt_pas_can_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_pt_pan_can_dpan;
   op_copy_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pt_p_caa_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pt_pas_caa_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_pt_pan_caa_dpan;

   op_copy_pt_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_copy_pt_p_c_dpas;
   op_copy_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_copy_pt_pas_c_dpas;
   op_copy_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_copy_pt_pan_c_dpas;
   op_copy_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_pt_p_can_dpas;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_pt_pas_can_dpas;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_pt_pan_can_dpas;
   op_copy_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_pt_p_caa_dpas;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_pt_pas_caa_dpas;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_pt_pan_caa_dpas;
}
#endif

/*-----*/

/* copy_rel pixel x color --> dst */

#ifdef BUILD_C
static void
_op_copy_rel_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 cs = MUL4_SYM(c, *s);

	*d = MUL_A_SYM(*d >> 24, cs) + (cs & 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_copy_rel_pas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;

	switch(a)
	  {
	    case 0:
		*d = MUL3_SYM(c, *s);
		break;
	    case 0xff000000:
		*d = MUL_A_256(ca, *d) + MUL3_SYM(c, *s);
		break;
	    default:
		a = MUL4_SYM(c, *s);
		*d = MUL_A_SYM(*d >> 24, a) + (a & 0x00ffffff);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_copy_rel_pan_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	*d = MUL_A_256(ca, *d) + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_copy_rel_p_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = MUL_A_SYM(*d >> 24, *s) + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_copy_rel_pas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		*d = MUL3_SYM(c, *s);
		break;
	    case 0xff000000:
		*d = (*d & 0xff000000) + MUL3_SYM(c, *s);
		break;
	    default:
		*d = MUL_A_SYM(*d >> 24, *s) + MUL3_SYM(c, *s);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_copy_rel_pan_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = (*d & 0xff000000) + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_copy_rel_p_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	DATA32  cs = (c * (*s >> 24)) >> 8;

	*d = MUL_A_SYM(cs, *d) + (*s & 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_copy_rel_pas_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	DATA32  a = *s & 0xff000000;

	switch(a)
	  {
	    case 0:
		*d = *s;
		break;
	    case 0xff000000:
		*d = MUL_A_256(c, *d) + (*s & 0x00ffffff);;
		break;
	    default:
		a = (c * (*s >> 24)) >> 8;
		*d = MUL_A_SYM(a, *d) + (*s & 0x00ffffff);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_copy_rel_pan_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	*d = MUL_A_256(c, *d) + (*s & 0x00ffffff);;
	s++;  d++;
     }
}

#define _op_copy_rel_p_c_dpan _op_copy_p_c_dpan
#define _op_copy_rel_pas_c_dpan _op_copy_pas_c_dpan
#define _op_copy_rel_pan_c_dpan _op_copy_pan_c_dpan
#define _op_copy_rel_p_can_dpan _op_copy_p_can_dpan
#define _op_copy_rel_pas_can_dpan _op_copy_pas_can_dpan
#define _op_copy_rel_pan_can_dpan _op_copy_pan_can_dpan
#define _op_copy_rel_p_caa_dpan _op_copy_p_caa_dpan
#define _op_copy_rel_pas_caa_dpan _op_copy_pas_caa_dpan
#define _op_copy_rel_pan_caa_dpan _op_copy_pan_caa_dpan

#define _op_copy_rel_p_c_dpas _op_copy_rel_p_c_dp
#define _op_copy_rel_pas_c_dpas _op_copy_rel_pas_c_dp
#define _op_copy_rel_pan_c_dpas _op_copy_rel_pan_c_dp
#define _op_copy_rel_p_can_dpas _op_copy_rel_p_can_dp
#define _op_copy_rel_pas_can_dpas _op_copy_rel_pas_can_dp
#define _op_copy_rel_pan_can_dpas _op_copy_rel_pan_can_dp
#define _op_copy_rel_p_caa_dpas _op_copy_rel_p_caa_dp
#define _op_copy_rel_pas_caa_dpas _op_copy_rel_pas_caa_dp
#define _op_copy_rel_pan_caa_dpas _op_copy_rel_pan_caa_dp

static void
init_copy_rel_pixel_color_span_funcs_c(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_rel_p_c_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_rel_pas_c_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_rel_pan_c_dp;
   op_copy_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_p_can_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pas_can_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pan_can_dp;
   op_copy_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_p_caa_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pas_caa_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pan_caa_dp;

   op_copy_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_p_c_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pas_c_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pan_c_dpan;
   op_copy_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_p_can_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pas_can_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pan_can_dpan;
   op_copy_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_p_caa_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pas_caa_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pan_caa_dpan;

   op_copy_rel_span_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_p_c_dpas;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_pas_c_dpas;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_pan_c_dpas;
   op_copy_rel_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_p_can_dpas;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_pas_can_dpas;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_pan_can_dpas;
   op_copy_rel_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_p_caa_dpas;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_pas_caa_dpas;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_pan_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_copy_rel_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL4_SYM(c, s);
	*d = MUL_A_SYM(*d >> 24, s) + (s & 0x00ffffff);
}

static void
_op_copy_rel_pt_pan_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(c >> 24, *d) + MUL3_SYM(c, s);
}

static void
_op_copy_rel_pt_p_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(*d >> 24, s) + MUL3_SYM(c, s);
}

static void
_op_copy_rel_pt_pan_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d & 0xff000000) + MUL3_SYM(c, s);
}

static void
_op_copy_rel_pt_p_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = (((c >> 24) * (s >> 24)) + 255) >> 8;
	*d = MUL_A_SYM(c, *d) + (s & 0x00ffffff);
}

static void
_op_copy_rel_pt_pan_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(c >> 24, *d) + (s & 0x00ffffff);;
}

#define _op_copy_rel_pt_pas_c_dp _op_copy_rel_pt_p_c_dp
#define _op_copy_rel_pt_pas_can_dp _op_copy_rel_pt_p_can_dp
#define _op_copy_rel_pt_pas_caa_dp _op_copy_rel_pt_p_caa_dp

#define _op_copy_rel_pt_p_c_dpan _op_copy_pt_p_c_dpan
#define _op_copy_rel_pt_pas_c_dpan _op_copy_pt_pas_c_dpan
#define _op_copy_rel_pt_pan_c_dpan _op_copy_pt_pan_c_dpan
#define _op_copy_rel_pt_p_can_dpan _op_copy_pt_p_can_dpan
#define _op_copy_rel_pt_pas_can_dpan _op_copy_pt_pas_can_dpan
#define _op_copy_rel_pt_pan_can_dpan _op_copy_pt_pan_can_dpan
#define _op_copy_rel_pt_p_caa_dpan _op_copy_pt_p_caa_dpan
#define _op_copy_rel_pt_pas_caa_dpan _op_copy_pt_pas_caa_dpan
#define _op_copy_rel_pt_pan_caa_dpan _op_copy_pt_pan_caa_dpan

#define _op_copy_rel_pt_p_c_dpas _op_copy_rel_pt_p_c_dp
#define _op_copy_rel_pt_pas_c_dpas _op_copy_rel_pt_pas_c_dp
#define _op_copy_rel_pt_pan_c_dpas _op_copy_rel_pt_pan_c_dp
#define _op_copy_rel_pt_p_can_dpas _op_copy_rel_pt_p_can_dp
#define _op_copy_rel_pt_pas_can_dpas _op_copy_rel_pt_pas_can_dp
#define _op_copy_rel_pt_pan_can_dpas _op_copy_rel_pt_pan_can_dp
#define _op_copy_rel_pt_p_caa_dpas _op_copy_rel_pt_p_caa_dp
#define _op_copy_rel_pt_pas_caa_dpas _op_copy_rel_pt_pas_caa_dp
#define _op_copy_rel_pt_pan_caa_dpas _op_copy_rel_pt_pan_caa_dp

static void
init_copy_rel_pixel_color_pt_funcs_c(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_copy_rel_pt_p_c_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_copy_rel_pt_pas_c_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_copy_rel_pt_pan_c_dp;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pt_p_can_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pt_pas_can_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_copy_rel_pt_pan_can_dp;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pt_p_caa_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pt_pas_caa_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_copy_rel_pt_pan_caa_dp;

   op_copy_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pt_p_c_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pt_pas_c_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_copy_rel_pt_pan_c_dpan;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_p_can_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_pas_can_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_copy_rel_pt_pan_can_dpan;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_p_caa_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_pas_caa_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_copy_rel_pt_pan_caa_dpan;

   op_copy_rel_pt_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_pt_p_c_dpas;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_pt_pas_c_dpas;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_copy_rel_pt_pan_c_dpas;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_pt_p_can_dpas;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_pt_pas_can_dpas;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_copy_rel_pt_pan_can_dpas;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_pt_p_caa_dpas;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_pt_pas_caa_dpas;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_copy_rel_pt_pan_caa_dpas;
}
#endif
