
/* blend pixel x color --> dst */

#ifdef BUILD_C
static void
_op_blend_p_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = MUL4_SYM(c, *s);
	DATA32 a = sc >> 24;
	DATA32 da = 1 + _evas_pow_lut[(a << 8) | (*d >> 24)];
	a++;
	*d = BLEND_ARGB_256(a, da, sc, *d);
	s++;  d++;
     }
}

static void
_op_blend_pas_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   DATA32 ca0, ca1;
   ca0 = c & 0xff000000;
   ca1 = 1 + (ca0 >> 24);
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
	      {
		DATA32 da = 1 + _evas_pow_lut[((ca0 >> 16) & 0xff00) | (*d >> 24)];
		a = ca0 + MUL3_SYM(c, *s);
		*d = BLEND_ARGB_256(ca1, da, a, *d);
	      }
		break;
	    default:
	      {
		DATA32 sc = MUL4_SYM(c, *s);
		DATA32 da = 1 + _evas_pow_lut[((sc >> 16) & 0xff00) | (*d >> 24)];
		a = 1 + (sc >> 24);
		*d = BLEND_ARGB_256(a, da, sc, *d);
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_c_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   DATA32 ca0, ca1;
   ca0 = c & 0xff000000;
   ca1 = 1 + (ca0 >> 24);
   while (d < e)
     {
	DATA32 da = 1 + _evas_pow_lut[((ca0 >> 16) & 0xff00) | (*d >> 24)];
	DATA32 sc = ca0 + MUL3_SYM(c, *s);
	*d = BLEND_ARGB_256(ca1, da, sc, *d);
	s++;  d++;
     }
}

static void
_op_blend_p_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = (*s & 0xff000000) + MUL3_SYM(c, *s);
	DATA32 da = 1 + _evas_pow_lut[((sc >> 16) & 0xff00) | (*d >> 24)];
	DATA32 a = 1 + (sc >> 24);
	*d = BLEND_ARGB_256(a, da, sc, *d);
	s++;  d++;
     }
}

static void
_op_blend_pas_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		*d = 0xff000000 + MUL3_SYM(c, *s);
		break;
	    default:
	      {
		DATA32 da = 1 + _evas_pow_lut[((a >> 16) & 0xff00) | (*d >> 24)];
		DATA32 sc = a + MUL3_SYM(c, *s);
		a = 1 + (a >> 24);
		*d = BLEND_ARGB_256(a, da, sc, *d);
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_can_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = 0xff000000 + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_blend_p_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
  while (d < e)
     {
	DATA32 sa = ((*s >> 24) * ca) >> 8;
	DATA32 da = 1 + _evas_pow_lut[(sa << 8) | (*d >> 24)];
	sa++;
	*d = BLEND_ARGB_256(sa, da, *s, *d);
	s++;  d++;
     }
}

static void
_op_blend_pas_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		a = 1 + _evas_pow_lut[((ca - 1) << 8) | (*d >> 24)];
		*d = BLEND_ARGB_256(ca, a, *s, *d);
		break;
	    default:
	      {
		DATA32 sa = ((a >> 24) * ca) >> 8;
		a = 1 + _evas_pow_lut[(sa << 8) | (*d >> 24)];
		sa++;
		*d = BLEND_ARGB_256(sa, a, *s, *d);
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_caa_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 da = 1 + _evas_pow_lut[((ca - 1) << 8) | (*d >> 24)];
	*d = BLEND_ARGB_256(ca, da, *s, *d);
	s++;  d++;
     }
}

static void
_op_blend_p_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = MUL4_SYM(c, *s);
	DATA32 a = 1 + (sc >> 24);
	*d = BLEND_RGB_256(a, sc, *d);
	s++;  d++;
     }
}

static void
_op_blend_pas_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   DATA32 ca0, ca1;
   ca0 = c & 0xff000000;
   ca1 = 1 + (ca0 >> 24);
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		a = ca0 + MUL3_SYM(c, *s);
		*d = BLEND_RGB_256(ca1, a, *d);
		break;
	    default:
	      {
		DATA32 sc = MUL4_SYM(c, *s);
		a = 1 + (sc >> 24);
		*d = BLEND_RGB_256(a, sc, *d);
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_c_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   DATA32 ca0, ca1;
   ca0 = c & 0xff000000;
   ca1 = 1 + (ca0 >> 24);
   while (d < e)
     {
	DATA32 sc = ca0 + MUL3_SYM(c, *s);
	*d = BLEND_RGB_256(ca1, sc, *d);
	s++;  d++;
     }
}

static void
_op_blend_p_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 sc = (*s & 0xff000000) + MUL3_SYM(c, *s);
	DATA32 a = 1 + (sc >> 24);
	*d = BLEND_RGB_256(a, sc, *d);
	s++;  d++;
     }
}

static void
_op_blend_pas_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		*d = 0xff000000 + MUL3_SYM(c, *s);
		break;
	    default:
	      {
		DATA32 sc = a + MUL3_SYM(c, *s);
		a = 1 + (a >> 24);
		*d = BLEND_RGB_256(a, sc, *d);
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_can_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	*d = 0xff000000 + MUL3_SYM(c, *s);
	s++;  d++;
     }
}

static void
_op_blend_p_caa_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
  while (d < e)
     {
	DATA32 sa = 1 + (((*s >> 24) * ca) >> 8);
	*d = BLEND_RGB_256(sa, *s, *d);
	s++;  d++;
     }
}

static void
_op_blend_pas_caa_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	DATA32 a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		*d = BLEND_RGB_256(ca, *s, *d);
		break;
	    default:
	      {
		DATA32 sa = 1 + (((a >> 24) * ca) >> 8);
		*d = BLEND_RGB_256(sa, *s, *d);
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_caa_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   while (d < e)
     {
	*d = BLEND_RGB_256(ca, *s, *d);
	s++;  d++;
     }
}

#define _op_blend_p_c_dpan _op_blend_p_c_dp
#define _op_blend_pas_c_dpan _op_blend_pas_c_dp
#define _op_blend_pan_c_dpan _op_blend_pan_c_dp
#define _op_blend_p_can_dpan _op_blend_p_can_dp
#define _op_blend_pas_can_dpan _op_blend_pas_can_dp
#define _op_blend_pan_can_dpan _op_blend_pan_can_dp
#define _op_blend_p_caa_dpan _op_blend_p_caa_dp
#define _op_blend_pas_caa_dpan _op_blend_pas_caa_dp
#define _op_blend_pan_caa_dpan _op_blend_pan_caa_dp

#define _op_blend_p_c_dpas _op_blend_p_c_dp
#define _op_blend_pas_c_dpas _op_blend_pas_c_dp
#define _op_blend_pan_c_dpas _op_blend_pan_c_dp
#define _op_blend_p_can_dpas _op_blend_p_can_dp
#define _op_blend_pas_can_dpas _op_blend_pas_can_dp
#define _op_blend_pan_can_dpas _op_blend_pan_can_dp
#define _op_blend_p_caa_dpas _op_blend_p_caa_dp
#define _op_blend_pas_caa_dpas _op_blend_pas_caa_dp
#define _op_blend_pan_caa_dpas _op_blend_pan_caa_dp

static void
init_blend_pixel_color_span_funcs_c(void)
{
   op_blend_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_blend_p_c_dp;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_blend_pas_c_dp;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_blend_pan_c_dp;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_blend_p_can_dp;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_blend_pas_can_dp;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_blend_pan_can_dp;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_blend_p_caa_dp;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_blend_pas_caa_dp;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_blend_pan_caa_dp;

   op_blend_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_blend_p_c_dpan;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_blend_pas_c_dpan;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_blend_pan_c_dpan;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_p_can_dpan;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_pas_can_dpan;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_pan_can_dpan;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_p_caa_dpan;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_pas_caa_dpan;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_pan_caa_dpan;

   op_blend_span_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_blend_p_c_dpas;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_blend_pas_c_dpas;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_blend_pan_c_dpas;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_p_can_dpas;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_pas_can_dpas;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_pan_can_dpas;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_p_caa_dpas;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_pas_caa_dpas;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_pan_caa_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_p_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 da;
	s = MUL4_SYM(c, s);
	c = s >> 24;
	da = 1 + _evas_pow_lut[(c << 8) + (*d >> 24)];
	*d = BLEND_ARGB_256(1 + c, da, s, *d);
}

static void
_op_blend_pt_pan_c_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 da = 1 + _evas_pow_lut[((c >> 16) & 0xff00) + (*d >> 24)];
	s = (c & 0xff000000) + MUL3_SYM(c, s);
	c = 1 + (c >> 24);
	*d = BLEND_ARGB_256(c, da, s, *d);
}

static void
_op_blend_pt_p_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 sc = (s & 0xff000000) + MUL3_SYM(c, s);
	c = 1 + _evas_pow_lut[((sc >> 16) & 0xff00) + (*d >> 24)];
	s = 1 + (sc >> 24);
	*d = BLEND_ARGB_256(s, c, sc, *d);
}

static void
_op_blend_pt_pan_can_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = 0xff000000 + MUL3_SYM(c, s);
}

static void
_op_blend_pt_p_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 sa = (((s >> 24) * (c >> 24)) + 255) >> 8;
	c = 1 + _evas_pow_lut[(sa << 8) + (*d >> 24)];
	*d = BLEND_ARGB_256(1 + sa, c, s, *d);
}

static void
_op_blend_pt_pan_caa_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 da = 1 + _evas_pow_lut[((c >> 16) & 0xff00) + (*d >> 24)];
	c = 1 + (c >> 24);
	*d = BLEND_ARGB_256(c, da, s, *d);
}

static void
_op_blend_pt_p_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = MUL4_SYM(c, s);
	c = 1 + (s >> 24);
	*d = BLEND_RGB_256(c, s, *d);
}

static void
_op_blend_pt_pan_c_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = (c & 0xff000000) + MUL3_SYM(c, s);
	c = 1 + (c >> 24);
	*d = BLEND_RGB_256(c, s, *d);
}

static void
_op_blend_pt_p_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = (s & 0xff000000) + MUL3_SYM(c, s);
	c = 1 + (s >> 24);
	*d = BLEND_RGB_256(c, s, *d);
}

static void
_op_blend_pt_pan_can_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = 0xff000000 + MUL3_SYM(c, s);
}

static void
_op_blend_pt_p_caa_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + ((((s >> 24) * (c >> 24)) + 255) >> 8);
	*d = BLEND_RGB_256(c, s, *d);
}

static void
_op_blend_pt_pan_caa_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + (c >> 24);
	*d = BLEND_RGB_256(c, s, *d);
}

#define _op_blend_pt_pas_c_dp _op_blend_pt_p_c_dp
#define _op_blend_pt_pas_can_dp _op_blend_pt_p_can_dp
#define _op_blend_pt_pas_caa_dp _op_blend_pt_p_caa_dp

#define _op_blend_pt_p_c_dpan _op_blend_pt_p_c_dp
#define _op_blend_pt_pas_c_dpan _op_blend_pt_pas_c_dp
#define _op_blend_pt_pan_c_dpan _op_blend_pt_pan_c_dp
#define _op_blend_pt_p_can_dpan _op_blend_pt_p_can_dp
#define _op_blend_pt_pas_can_dpan _op_blend_pt_pas_can_dp
#define _op_blend_pt_pan_can_dpan _op_blend_pt_pan_can_dp
#define _op_blend_pt_p_caa_dpan _op_blend_pt_p_caa_dp
#define _op_blend_pt_pas_caa_dpan _op_blend_pt_pas_caa_dp
#define _op_blend_pt_pan_caa_dpan _op_blend_pt_pan_caa_dp

#define _op_blend_pt_p_c_dpas _op_blend_pt_p_c_dp
#define _op_blend_pt_pas_c_dpas _op_blend_pt_pas_c_dp
#define _op_blend_pt_pan_c_dpas _op_blend_pt_pan_c_dp
#define _op_blend_pt_p_can_dpas _op_blend_pt_p_can_dp
#define _op_blend_pt_pas_can_dpas _op_blend_pt_pas_can_dp
#define _op_blend_pt_pan_can_dpas _op_blend_pt_pan_can_dp
#define _op_blend_pt_p_caa_dpas _op_blend_pt_p_caa_dp
#define _op_blend_pt_pas_caa_dpas _op_blend_pt_pas_caa_dp
#define _op_blend_pt_pan_caa_dpas _op_blend_pt_pan_caa_dp

static void
init_blend_pixel_color_pt_funcs_c(void)
{
   op_blend_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_blend_pt_p_c_dp;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_blend_pt_pas_c_dp;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_blend_pt_pan_c_dp;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_blend_pt_p_can_dp;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_blend_pt_pas_can_dp;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_blend_pt_pan_can_dp;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_blend_pt_p_caa_dp;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_blend_pt_pas_caa_dp;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_blend_pt_pan_caa_dp;

   op_blend_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_blend_pt_p_c_dpan;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_blend_pt_pas_c_dpan;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_blend_pt_pan_c_dpan;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_pt_p_can_dpan;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_pt_pas_can_dpan;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_pt_pan_can_dpan;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_pt_p_caa_dpan;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_pt_pas_caa_dpan;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_pt_pan_caa_dpan;

   op_blend_pt_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_blend_pt_p_c_dpas;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_blend_pt_pas_c_dpas;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_blend_pt_pan_c_dpas;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_pt_p_can_dpas;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_pt_pas_can_dpas;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_pt_pan_can_dpas;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_pt_p_caa_dpas;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_pt_pas_caa_dpas;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_pt_pan_caa_dpas;
}
#endif

/*-----*/

/* blend_rel pixel x color -> dst */

#ifdef BUILD_C
#define _op_blend_rel_p_c_dp _op_blend_p_c_dpan
#define _op_blend_rel_pas_c_dp _op_blend_pas_c_dpan
#define _op_blend_rel_pan_c_dp _op_blend_pan_c_dpan
#define _op_blend_rel_p_can_dp _op_blend_p_can_dpan
#define _op_blend_rel_pas_can_dp _op_blend_pas_can_dpan
#define _op_blend_rel_pan_can_dp _op_blend_pan_can_dpan
#define _op_blend_rel_p_caa_dp _op_blend_p_caa_dpan
#define _op_blend_rel_pas_caa_dp _op_blend_pas_caa_dpan
#define _op_blend_rel_pan_caa_dp _op_blend_pan_caa_dpan

#define _op_blend_rel_p_c_dpan _op_blend_p_c_dpan
#define _op_blend_rel_pas_c_dpan _op_blend_pas_c_dpan
#define _op_blend_rel_pan_c_dpan _op_blend_pan_c_dpan
#define _op_blend_rel_p_can_dpan _op_blend_p_can_dpan
#define _op_blend_rel_pas_can_dpan _op_blend_pas_can_dpan
#define _op_blend_rel_pan_can_dpan _op_blend_pan_can_dpan
#define _op_blend_rel_p_caa_dpan _op_blend_p_caa_dpan
#define _op_blend_rel_pas_caa_dpan _op_blend_pas_caa_dpan
#define _op_blend_rel_pan_caa_dpan _op_blend_pan_caa_dpan

#define _op_blend_rel_p_c_dpas _op_blend_rel_p_c_dp
#define _op_blend_rel_pas_c_dpas _op_blend_rel_pas_c_dp
#define _op_blend_rel_pan_c_dpas _op_blend_rel_pan_c_dp
#define _op_blend_rel_p_can_dpas _op_blend_rel_p_can_dp
#define _op_blend_rel_pas_can_dpas _op_blend_rel_pas_can_dp
#define _op_blend_rel_pan_can_dpas _op_blend_rel_pan_can_dp
#define _op_blend_rel_p_caa_dpas _op_blend_rel_p_caa_dp
#define _op_blend_rel_pas_caa_dpas _op_blend_rel_pas_caa_dp
#define _op_blend_rel_pan_caa_dpas _op_blend_rel_pan_caa_dp

static void
init_blend_rel_pixel_color_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC][DP][CPU_C] = _op_blend_rel_p_c_dp;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_blend_rel_pas_c_dp;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_blend_rel_pan_c_dp;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_blend_rel_p_can_dp;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_blend_rel_pas_can_dp;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_blend_rel_pan_can_dp;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_p_caa_dp;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_pas_caa_dp;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_pan_caa_dp;

   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_p_c_dpan;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_pas_c_dpan;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_pan_c_dpan;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_rel_p_can_dpan;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_rel_pas_can_dpan;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_rel_pan_can_dpan;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_p_caa_dpan;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pas_caa_dpan;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pan_caa_dpan;

   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_p_c_dpas;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_pas_c_dpas;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_pan_c_dpas;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_rel_p_can_dpas;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_rel_pas_can_dpas;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_rel_pan_can_dpas;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_p_caa_dpas;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_pas_caa_dpas;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_pan_caa_dpas;
}
#endif

#ifdef BUILD_C
#define _op_blend_rel_pt_p_c_dp _op_blend_pt_p_c_dpan
#define _op_blend_rel_pt_pas_c_dp _op_blend_pt_pas_c_dpan
#define _op_blend_rel_pt_pan_c_dp _op_blend_pt_pan_c_dpan
#define _op_blend_rel_pt_p_can_dp _op_blend_pt_p_can_dpan
#define _op_blend_rel_pt_pas_can_dp _op_blend_pt_pas_can_dpan
#define _op_blend_rel_pt_pan_can_dp _op_blend_pt_pan_can_dpan
#define _op_blend_rel_pt_p_caa_dp _op_blend_pt_p_caa_dpan
#define _op_blend_rel_pt_pas_caa_dp _op_blend_pt_pas_caa_dpan
#define _op_blend_rel_pt_pan_caa_dp _op_blend_pt_pan_caa_dpan

#define _op_blend_rel_pt_p_c_dpan _op_blend_pt_p_c_dpan
#define _op_blend_rel_pt_pas_c_dpan _op_blend_pt_pas_c_dpan
#define _op_blend_rel_pt_pan_c_dpan _op_blend_pt_pan_c_dpan
#define _op_blend_rel_pt_p_can_dpan _op_blend_pt_p_can_dpan
#define _op_blend_rel_pt_pas_can_dpan _op_blend_pt_pas_can_dpan
#define _op_blend_rel_pt_pan_can_dpan _op_blend_pt_pan_can_dpan
#define _op_blend_rel_pt_p_caa_dpan _op_blend_pt_p_caa_dpan
#define _op_blend_rel_pt_pas_caa_dpan _op_blend_pt_pas_caa_dpan
#define _op_blend_rel_pt_pan_caa_dpan _op_blend_pt_pan_caa_dpan

#define _op_blend_rel_pt_p_c_dpas _op_blend_rel_pt_p_c_dp
#define _op_blend_rel_pt_pas_c_dpas _op_blend_rel_pt_pas_c_dp
#define _op_blend_rel_pt_pan_c_dpas _op_blend_rel_pt_pan_c_dp
#define _op_blend_rel_pt_p_can_dpas _op_blend_rel_pt_p_can_dp
#define _op_blend_rel_pt_pas_can_dpas _op_blend_rel_pt_pas_can_dp
#define _op_blend_rel_pt_pan_can_dpas _op_blend_rel_pt_pan_can_dp
#define _op_blend_rel_pt_p_caa_dpas _op_blend_rel_pt_p_caa_dp
#define _op_blend_rel_pt_pas_caa_dpas _op_blend_rel_pt_pas_caa_dp
#define _op_blend_rel_pt_pan_caa_dpas _op_blend_rel_pt_pan_caa_dp

static void
init_blend_rel_pixel_color_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC][DP][CPU_C] = _op_blend_rel_pt_p_c_dp;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP][CPU_C] = _op_blend_rel_pt_pas_c_dp;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP][CPU_C] = _op_blend_rel_pt_pan_c_dp;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP][CPU_C] = _op_blend_rel_pt_p_can_dp;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_C] = _op_blend_rel_pt_pas_can_dp;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_C] = _op_blend_rel_pt_pan_can_dp;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_pt_p_caa_dp;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_pt_pas_caa_dp;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_C] = _op_blend_rel_pt_pan_caa_dp;

   op_blend_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_pt_p_c_dpan;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_pt_pas_c_dpan;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_C] = _op_blend_rel_pt_pan_c_dpan;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_rel_pt_p_can_dpan;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_rel_pt_pas_can_dpan;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_C] = _op_blend_rel_pt_pan_can_dpan;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pt_p_caa_dpan;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pt_pas_caa_dpan;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_C] = _op_blend_rel_pt_pan_caa_dpan;

   op_blend_rel_pt_funcs[SP][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_pt_p_c_dpas;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_pt_pas_c_dpas;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_C] = _op_blend_rel_pt_pan_c_dpas;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_rel_pt_p_can_dpas;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_rel_pt_pas_can_dpas;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_C] = _op_blend_rel_pt_pan_can_dpas;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_pt_p_caa_dpas;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_pt_pas_caa_dpas;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_C] = _op_blend_rel_pt_pan_caa_dpas;
}
#endif
