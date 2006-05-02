
/* add pixel x mask --> dst */

#ifdef BUILD_C
static void
_op_add_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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
		DATA32 saa = 1 + _evas_pow_lut[((*s >> 16) & 0xff00) + (*d >> 24)];
		DATA32 daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (*s >> 24)];
		MUL2_ADD_256(saa, daa, *s, *d, *d)
	      }
		break;
	    default:
	      {
		DATA32 saa, daa;
		a = MUL_A_SYM(a, *s) + (*s & 0x00ffffff);
		saa = 1 + _evas_pow_lut[((a >> 16) & 0xff00) + (*d >> 24)];
		daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (a >> 24)];
		MUL2_ADD_256(saa, daa, a, *d, *d)
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_add_pas_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch((*s >> 24) & a)
	  {
	    case 0:
		break;
	    case 255:
		MUL_ADD_SYM(*d >> 24, *d, *s, *d)
		break;
	    default:
	      {
		DATA32 saa, daa;
		a = MUL_A_SYM(a, *s) + (*s & 0x00ffffff);
		saa = 1 + _evas_pow_lut[((a >> 16) & 0xff00) + (*d >> 24)];
		daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (a >> 24)];
		MUL2_ADD_256(saa, daa, a, *d, *d)
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_add_pan_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		MUL_ADD_SYM(*d >> 24, *d, *s, *d)
		break;
	    default:
	      {
		DATA32 saa, daa;
		a = (a << 24) + (*s & 0x00ffffff);
		saa = 1 + _evas_pow_lut[((a >> 16) & 0xff00) + (*d >> 24)];
		daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (a >> 24)];
		MUL2_ADD_256(saa, daa, a, *d, *d)
	      }
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_add_p_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		MUL_ADD_SYM(*s >> 24, *s, *d, *d)
		break;
	    default:
		a = 1 + (((a * (*s >> 24)) + 255) >> 8);
		MUL_ADD_256(a, *s, *d, *d)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_add_pas_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = (*s >> 24);
	switch(*m & a)
	  {
	    case 0:
		break;
	    case 255:
		ADD(*s, *d, *d)
		break;
	    default:
		a = 1 + (((*m * a) + 255) >> 8);
		MUL_ADD_256(a, *s, *d, *d)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_add_pan_mas_dpan(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*m)
	  {
	    case 0:
		break;
	    case 255:
		ADD(*s, *d, *d)
		break;
	    default:
		MUL_ADD_SYM(*m, *s, *d, *d)
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_add_p_mas_dpas _op_add_p_mas_dp
#define _op_add_pas_mas_dpas _op_add_pas_mas_dp
#define _op_add_pan_mas_dpas _op_add_pan_mas_dp

static void
init_add_pixel_mask_span_funcs_c(void)
{
   op_add_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_add_p_mas_dp;
   op_add_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_add_pas_mas_dp;
   op_add_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_add_pan_mas_dp;

   op_add_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_p_mas_dpan;
   op_add_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_pas_mas_dpan;
   op_add_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_pan_mas_dpan;

   op_add_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_p_mas_dpas;
   op_add_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_pas_mas_dpas;
   op_add_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_pan_mas_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_add_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 saa, daa;
	s = MUL_A_SYM(m, s) + (s & 0x00ffffff);
	saa = 1 + _evas_pow_lut[((s >> 16) & 0xff00) + (*d >> 24)];
	daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + (s >> 24)];
	MUL2_ADD_256(saa, daa, s, *d, *d)
}

static void
_op_add_pt_pan_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	DATA32 saa, daa;
	s = (m << 24) + (s & 0x00ffffff);
	saa = 1 + _evas_pow_lut[(m << 8) + (*d >> 24)];
	daa = 1 + _evas_pow_lut[((*d >> 16) & 0xff00) + m];
	MUL2_ADD_256(saa, daa, s, *d, *d)
}

static void
_op_add_pt_p_mas_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + (((m * (s >> 24)) + 255) >> 8);
	MUL_ADD_256(c, s, *d, *d)
}

static void
_op_add_pt_pan_mas_dpan(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_ADD_SYM(m, s, *d, *d)
}

#define _op_add_pt_pas_mas_dp _op_add_pt_p_mas_dp

#define _op_add_pt_pas_mas_dpan _op_add_pt_p_mas_dpan

#define _op_add_pt_p_mas_dpas _op_add_pt_p_mas_dp
#define _op_add_pt_pas_mas_dpas _op_add_pt_pas_mas_dp
#define _op_add_pt_pan_mas_dpas _op_add_pt_pan_mas_dp

static void
init_add_pixel_mask_pt_funcs_c(void)
{
   op_add_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_add_pt_p_mas_dp;
   op_add_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_add_pt_pas_mas_dp;
   op_add_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_add_pt_pan_mas_dp;

   op_add_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_pt_p_mas_dpan;
   op_add_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_pt_pas_mas_dpan;
   op_add_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_pt_pan_mas_dpan;

   op_add_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_pt_p_mas_dpas;
   op_add_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_pt_pas_mas_dpas;
   op_add_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_pt_pan_mas_dpas;
}
#endif

/*-----*/

/* add_rel pixel x mask --> dst */

#ifdef BUILD_C
static void
_op_add_rel_p_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		MUL_ADD_REL_SYM(*s >> 24, *s, *d, *d)
		break;
	    default:
		a = 1 + (((a * (*s >> 24)) + 255) >> 8);
		MUL_ADD_REL_256(a, *s, *d, *d)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_add_rel_pas_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	DATA32 a = (*s >> 24);
	switch(*m & a)
	  {
	    case 0:
		break;
	    case 255:
		ADD_REL(*s, *d, *d)
		break;
	    default:
		a = 1 + (((*m * a) + 255) >> 8);
		MUL_ADD_REL_256(a, *s, *d, *d)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_add_rel_pan_mas_dp(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   while (d < e)
     {
	switch(*m)
	  {
	    case 0:
		break;
	    case 255:
		ADD_REL(*s, *d, *d)
		break;
	    default:
		MUL_ADD_REL_SYM(*m, *s, *d, *d)
		break;
	  }
	m++;  s++;  d++;
     }
}


#define _op_add_rel_p_mas_dpan _op_add_p_mas_dpan
#define _op_add_rel_pas_mas_dpan _op_add_pas_mas_dpan
#define _op_add_rel_pan_mas_dpan _op_add_pan_mas_dpan

#define _op_add_rel_p_mas_dpas _op_add_rel_p_mas_dp
#define _op_add_rel_pas_mas_dpas _op_add_rel_pas_mas_dp
#define _op_add_rel_pan_mas_dpas _op_add_rel_pan_mas_dp

static void
init_add_rel_pixel_mask_span_funcs_c(void)
{
   op_add_rel_span_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_p_mas_dp;
   op_add_rel_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_pas_mas_dp;
   op_add_rel_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_pan_mas_dp;

   op_add_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_p_mas_dpan;
   op_add_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_pas_mas_dpan;
   op_add_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_pan_mas_dpan;

   op_add_rel_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_p_mas_dpas;
   op_add_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_pas_mas_dpas;
   op_add_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_pan_mas_dpas;
}
#endif

#ifdef BUILD_C
static void
_op_add_rel_pt_p_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + (((m * (s >> 24)) + 255) >> 8);
	MUL_ADD_REL_256(c, s, *d, *d)
}

static void
_op_add_rel_pt_pan_mas_dp(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MUL_ADD_REL_SYM(m, s, *d, *d)
}

#define _op_add_rel_pt_pas_mas_dp _op_add_rel_pt_p_mas_dp

#define _op_add_rel_pt_p_mas_dpan _op_add_pt_p_mas_dpan
#define _op_add_rel_pt_pas_mas_dpan _op_add_pt_pas_mas_dpan
#define _op_add_rel_pt_pan_mas_dpan _op_add_pt_pan_mas_dpan

#define _op_add_rel_pt_p_mas_dpas _op_add_rel_pt_p_mas_dp
#define _op_add_rel_pt_pas_mas_dpas _op_add_rel_pt_pas_mas_dp
#define _op_add_rel_pt_pan_mas_dpas _op_add_rel_pt_pan_mas_dp

static void
init_add_rel_pixel_mask_pt_funcs_c(void)
{
   op_add_rel_pt_funcs[SP][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_pt_p_mas_dp;
   op_add_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_pt_pas_mas_dp;
   op_add_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_C] = _op_add_rel_pt_pan_mas_dp;

   op_add_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_pt_p_mas_dpan;
   op_add_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_pt_pas_mas_dpan;
   op_add_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_C] = _op_add_rel_pt_pan_mas_dpan;

   op_add_rel_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_pt_p_mas_dpas;
   op_add_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_pt_pas_mas_dpas;
   op_add_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_C] = _op_add_rel_pt_pan_mas_dpas;
}
#endif
