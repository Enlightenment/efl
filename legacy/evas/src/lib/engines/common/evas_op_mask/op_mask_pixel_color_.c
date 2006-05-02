
/* mask pixel x color --> dst */

#ifdef BUILD_C
static void
_op_mask_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 cs = 1 + ((c * (*s >> 24)) >> 8);
	*d = MUL_A_256(cs, *d) + (*d & 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_mask_pas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		*d &= 0x00ffffff;
		break;
	    case 0xff000000:
		*d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
		break;
	    default:
		a = 1 + ((c * (*s >> 24)) >> 8);
		*d = MUL_A_256(a, *d) + (*d & 0x00ffffff);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_mask_pan_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	*d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_mask_p_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = MUL_A_SYM(*s >> 24, *d) + (*d & 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_mask_pas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		*d &= 0x00ffffff;
		break;
	    case 0xff000000:
		break;
	    default:
		*d = MUL_A_SYM(*s >> 24, *d) + (*d & 0x00ffffff);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_mask_p_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   while (d < e)
     {
	*d = MUL_A_256(c, *s) + (*d & 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_mask_pas_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   c |= 0x00ffffff;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		*d &= 0x00ffffff;
		break;
	    case 0xff000000:
		*d &= c;
		break;
	    default:
		*d = MUL_A_256(ca, *s) + (*d & 0x00ffffff);
		break;
	  }
	s++;  d++;
     }
}

static void
_op_mask_pan_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c |= 0x00ffffff;
   while (d < e)
     {
	*d &= c;
	s++;  d++;
     }
}

static void
_op_mask_p_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d &= (*s | 0x00ffffff);
	s++;  d++;
     }
}

static void
_op_mask_pas_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		*d &= 0x00ffffff;
		break;
	    case 0xff000000:
		break;
	    default:
		*d &= (*s | 0x00ffffff);
		break;
	  }
	s++;  d++;
     }
}

#define _op_mask_pan_can_dp NULL
#define _op_mask_p_caa_dp _op_mask_p_c_dp
#define _op_mask_pas_caa_dp _op_mask_pas_c_dp
#define _op_mask_pan_caa_dp _op_mask_pan_c_dp

#define _op_mask_pan_can_dpan _op_mask_pan_can_dp
#define _op_mask_p_caa_dpan _op_mask_p_c_dpan
#define _op_mask_pas_caa_dpan _op_mask_pas_c_dpan
#define _op_mask_pan_caa_dpan _op_mask_pan_c_dpan

#define _op_mask_p_c_dpas _op_mask_p_c_dp
#define _op_mask_pas_c_dpas _op_mask_pas_c_dp
#define _op_mask_pan_c_dpas _op_mask_pan_c_dp
#define _op_mask_p_can_dpas _op_mask_p_can_dp
#define _op_mask_pas_can_dpas _op_mask_pas_can_dp
#define _op_mask_pan_can_dpas _op_mask_pan_can_dp
#define _op_mask_p_caa_dpas _op_mask_p_caa_dp
#define _op_mask_pas_caa_dpas _op_mask_pas_caa_dp
#define _op_mask_pan_caa_dpas _op_mask_pan_caa_dp

static void
init_mask_pixel_color_span_funcs_c(void)
{
   op_mask_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_mask_p_c_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_mask_pas_c_dp;
   op_mask_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_mask_pan_c_dp;
   op_mask_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_mask_p_can_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_mask_pas_can_dp;
   op_mask_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_mask_pan_can_dp;
   op_mask_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_mask_p_caa_dp;
   op_mask_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_mask_pas_caa_dp;
   op_mask_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_mask_pan_caa_dp;

   op_mask_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_mask_p_c_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_mask_pas_c_dpan;
   op_mask_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_mask_pan_c_dpan;
   op_mask_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_p_can_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pas_can_dpan;
   op_mask_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pan_can_dpan;
   op_mask_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_p_caa_dpan;
   op_mask_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pas_caa_dpan;
   op_mask_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pan_caa_dpan;

   op_mask_span_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_mask_p_c_dpas;
   op_mask_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_mask_pas_c_dpas;
   op_mask_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_mask_pan_c_dpas;
   op_mask_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_p_can_dpas;
   op_mask_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_pas_can_dpas;
   op_mask_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_pan_can_dpas;
   op_mask_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_p_caa_dpas;
   op_mask_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_pas_caa_dpas;
   op_mask_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_pan_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_mask_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + ((((c >> 24) * (s >> 24)) + 255) >> 8);
	*d = MUL_A_256(c, *d) + (*d & 0x00ffffff);
}

static void
_op_mask_pt_pan_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(c >> 24, *d) + (*d & 0x00ffffff);
}

static void
_op_mask_pt_p_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(s >> 24, *d) + (*d & 0x00ffffff);
}

static void
_op_mask_pt_p_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = MUL_A_SYM(c >> 24, s) + (*d & 0x00ffffff);
}

static void
_op_mask_pt_pan_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d &= (c | 0x00ffffff);
}

static void
_op_mask_pt_p_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d &= (s | 0x00ffffff);
}

#define _op_mask_pt_pas_c_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_can_dp _op_mask_pt_p_can_dp
#define _op_mask_pt_pan_can_dp NULL
#define _op_mask_pt_p_caa_dp _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_caa_dp _op_mask_pt_pas_c_dp
#define _op_mask_pt_pan_caa_dp _op_mask_pt_pan_c_dp

#define _op_mask_pt_pas_c_dpan _op_mask_pt_p_c_dpan
#define _op_mask_pt_pas_can_dpan _op_mask_pt_p_can_dpan
#define _op_mask_pt_pan_can_dpan _op_mask_pt_pan_can_dp
#define _op_mask_pt_p_caa_dpan _op_mask_pt_p_c_dpan
#define _op_mask_pt_pas_caa_dpan _op_mask_pt_pas_c_dpan
#define _op_mask_pt_pan_caa_dpan _op_mask_pt_pan_c_dpan

#define _op_mask_pt_p_c_dpas _op_mask_pt_p_c_dp
#define _op_mask_pt_pas_c_dpas _op_mask_pt_pas_c_dp
#define _op_mask_pt_pan_c_dpas _op_mask_pt_pan_c_dp
#define _op_mask_pt_p_can_dpas _op_mask_pt_p_can_dp
#define _op_mask_pt_pas_can_dpas _op_mask_pt_pas_can_dp
#define _op_mask_pt_pan_can_dpas _op_mask_pt_pan_can_dp
#define _op_mask_pt_p_caa_dpas _op_mask_pt_p_caa_dp
#define _op_mask_pt_pas_caa_dpas _op_mask_pt_pas_caa_dp
#define _op_mask_pt_pan_caa_dpas _op_mask_pt_pan_caa_dp

static void
init_mask_pixel_color_pt_funcs_c(void)
{
   op_mask_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_mask_pt_p_c_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_mask_pt_pas_c_dp;
   op_mask_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_mask_pt_pan_c_dp;
   op_mask_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_mask_pt_p_can_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_mask_pt_pas_can_dp;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_mask_pt_pan_can_dp;
   op_mask_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_p_caa_dp;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_pas_caa_dp;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_mask_pt_pan_caa_dp;

   op_mask_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_p_c_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_pas_c_dpan;
   op_mask_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_mask_pt_pan_c_dpan;
   op_mask_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pt_p_can_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pt_pas_can_dpan;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_mask_pt_pan_can_dpan;
   op_mask_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_p_caa_dpan;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_pas_caa_dpan;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_mask_pt_pan_caa_dpan;

   op_mask_pt_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_mask_pt_p_c_dpas;
   op_mask_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_mask_pt_pas_c_dpas;
   op_mask_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_mask_pt_pan_c_dpas;
   op_mask_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_pt_p_can_dpas;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_pt_pas_can_dpas;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_mask_pt_pan_can_dpas;
   op_mask_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_pt_p_caa_dpas;
   op_mask_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_pt_pas_caa_dpas;
   op_mask_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_mask_pt_pan_caa_dpas;
}
#endif
