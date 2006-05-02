
/* blend pixel x mask --> dst */

#ifdef BUILD_MMX
static void
_op_blend_p_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		a = *s & 0xff000000;
		MOV_MA2R(a, mm2)
		psllq_i2r(48, mm2);
		a = _evas_pow_lut[(a >> 16) + (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);

		por_r2r(mm2, mm3);
		psrlw_i2r(1, mm3);

		a = *s | 0xff000000;
		MOV_P2R(a, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	    default:
		MOV_A2R(a, mm3)
		a = 1 + (*s >> 24);
		MOV_A2R(a, mm4)
		MUL4_256_R2R(mm4, mm3)
		MOV_R2P(mm3, a, mm0)
		MOV_RA2R(mm3, mm2)
		psllq_i2r(48, mm2);

		a = _evas_pow_lut[(a & 0xff00) + (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);

		por_r2r(mm2, mm3);
		psrlw_i2r(1, mm3);

		a = *s | 0xff000000;
		MOV_P2R(a, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pas_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
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
		MOV_A2R(a, mm3)
		a = 1 + *m;
		MOV_A2R(a, mm4)
		MUL4_256_R2R(mm4, mm3)
		MOV_R2P(mm3, a, mm0)
		MOV_RA2R(mm3, mm2)
		psllq_i2r(48, mm2);

		a = _evas_pow_lut[(a & 0xff00) + (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);

		por_r2r(mm2, mm3);
		psrlw_i2r(1, mm3);

		a = *s | 0xff000000;
		MOV_P2R(a, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pan_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
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
		MOV_MA2R(a, mm2)
		psllq_i2r(48, mm2);

		a = _evas_pow_lut[(a << 8) + (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);

		por_r2r(mm2, mm3);
		psrlw_i2r(1, mm3);

		MOV_P2R(*s, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_p_mas_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		movd_m2r(*s, mm1);

		MOV_RA2R(mm1, mm3)
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	    default:
		MOV_A2R(a, mm3)
		a = 1 + (*s >> 24);
		MOV_A2R(a, mm4)
		MUL4_256_R2R(mm4, mm3)
		movq_r2r(mm3, mm4);
		psllw_i2r(8, mm4);
		por_r2r(mm4, mm3);
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		MOV_P2R(*s, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pas_mas_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
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
		MOV_A2R(a, mm3)
		a = 1 + *m;
		MOV_A2R(a, mm4)
		MUL4_256_R2R(mm4, mm3)
		movq_r2r(mm3, mm4);
		psllw_i2r(8, mm4);
		por_r2r(mm4, mm3);
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		MOV_P2R(*s, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pan_mas_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
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
		a <<= 24;
		MOV_MA2R(a, mm3)
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		MOV_P2R(*s, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_blend_p_mas_dpas_mmx _op_blend_p_mas_dp_mmx
#define _op_blend_pan_mas_dpas_mmx _op_blend_pan_mas_dp_mmx
#define _op_blend_pas_mas_dpas_mmx _op_blend_pas_mas_dp_mmx

static void
init_blend_pixel_mask_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_p_mas_dp_mmx;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pas_mas_dp_mmx;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pan_mas_dp_mmx;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_p_mas_dpan_mmx;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pas_mas_dpan_mmx;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pan_mas_dpan_mmx;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_p_mas_dpas_mmx;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_pas_mas_dpas_mmx;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_pan_mas_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_p_mas_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)
	MOV_A2R(m, mm3)
	c = 1 + (s >> 24);
	MOV_A2R(c, mm4)
	MUL4_256_R2R(mm4, mm3)
	MOV_R2P(mm3, c, mm0)
	MOV_RA2R(mm3, mm2)
	psllq_i2r(48, mm2);

	c = _evas_pow_lut[(c & 0xff00) + (*d >> 24)] << 24;
	MOV_MA2R(c, mm3)
	psrlq_i2r(16, mm3);

	por_r2r(mm2, mm3);
	psrlw_i2r(1, mm3);

	s |= 0xff000000;
	MOV_P2R(s, mm1, mm0)
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

static void
_op_blend_pt_pan_mas_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)
	MOV_MA2R(m, mm2)
	psllq_i2r(48, mm2);

	c = _evas_pow_lut[(m << 8) + (*d >> 24)] << 24;
	MOV_MA2R(c, mm3)
	psrlq_i2r(16, mm3);
	por_r2r(mm2, mm3);
	psrlw_i2r(1, mm3);

	MOV_P2R(s, mm1, mm0)
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

static void
_op_blend_pt_p_mas_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)
	MOV_P2R(s, mm1, mm0)
	s = 1 + (s >> 24);
	MOV_A2R(s, mm4)
	MOV_A2R(m, mm3)
	MUL4_256_R2R(mm4, mm3)
	packuswb_r2r(mm0, mm3);
	punpcklbw_r2r(mm3, mm3);
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

static void
_op_blend_pt_pan_mas_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)

	MOV_P2R(s, mm1, mm0)
	s = m << 24;
	MOV_MA2R(s, mm3)
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

#define _op_blend_pt_pas_mas_dp_mmx _op_blend_pt_p_mas_dp_mmx

#define _op_blend_pt_pas_mas_dpan_mmx _op_blend_pt_p_mas_dpan_mmx

#define _op_blend_pt_p_mas_dpas_mmx _op_blend_pt_p_mas_dp_mmx
#define _op_blend_pt_pan_mas_dpas_mmx _op_blend_pt_pan_mas_dp_mmx
#define _op_blend_pt_pas_mas_dpas_mmx _op_blend_pt_pas_mas_dp_mmx

static void
init_blend_pixel_mask_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_p_mas_dp_mmx;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_pas_mas_dp_mmx;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_pan_mas_dp_mmx;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_p_mas_dpan_mmx;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_pas_mas_dpan_mmx;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_pan_mas_dpan_mmx;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_pt_p_mas_dpas_mmx;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_pt_pas_mas_dpas_mmx;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_pt_pan_mas_dpas_mmx;
}
#endif

/*-----*/

/* blend_rel pixel x mask -> dst */

#ifdef BUILD_MMX
#define _op_blend_rel_p_mas_dp_mmx _op_blend_p_mas_dpan_mmx
#define _op_blend_rel_pas_mas_dp_mmx _op_blend_pas_mas_dpan_mmx
#define _op_blend_rel_pan_mas_dp_mmx _op_blend_pan_mas_dpan_mmx

#define _op_blend_rel_p_mas_dpan_mmx _op_blend_p_mas_dpan_mmx
#define _op_blend_rel_pas_mas_dpan_mmx _op_blend_pas_mas_dpan_mmx
#define _op_blend_rel_pan_mas_dpan_mmx _op_blend_pan_mas_dpan_mmx

#define _op_blend_rel_p_mas_dpas_mmx _op_blend_rel_p_mas_dp_mmx
#define _op_blend_rel_pas_mas_dpas_mmx _op_blend_rel_pas_mas_dp_mmx
#define _op_blend_rel_pan_mas_dpas_mmx _op_blend_rel_pan_mas_dp_mmx

static void
init_blend_rel_pixel_mask_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_p_mas_dp_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_pas_mas_dp_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_pan_mas_dp_mmx;

   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_p_mas_dpan_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pas_mas_dpan_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pan_mas_dpan_mmx;

   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_p_mas_dpas_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pas_mas_dpas_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pan_mas_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
#define _op_blend_rel_pt_p_mas_dp_mmx _op_blend_pt_p_mas_dpan_mmx
#define _op_blend_rel_pt_pas_mas_dp_mmx _op_blend_pt_pas_mas_dpan_mmx
#define _op_blend_rel_pt_pan_mas_dp_mmx _op_blend_pt_pan_mas_dpan_mmx

#define _op_blend_rel_pt_p_mas_dpan_mmx _op_blend_pt_p_mas_dpan_mmx
#define _op_blend_rel_pt_pas_mas_dpan_mmx _op_blend_pt_pas_mas_dpan_mmx
#define _op_blend_rel_pt_pan_mas_dpan_mmx _op_blend_pt_pan_mas_dpan_mmx

#define _op_blend_rel_pt_p_mas_dpas_mmx _op_blend_rel_pt_p_mas_dp_mmx
#define _op_blend_rel_pt_pas_mas_dpas_mmx _op_blend_rel_pt_pas_mas_dp_mmx
#define _op_blend_rel_pt_pan_mas_dpas_mmx _op_blend_rel_pt_pan_mas_dp_mmx

static void
init_blend_rel_pixel_mask_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_p_mas_dp_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_pas_mas_dp_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_pan_mas_dp_mmx;

   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_p_mas_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_pas_mas_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_pan_mas_dpan_mmx;

   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pt_p_mas_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pt_pas_mas_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pt_pan_mas_dpas_mmx;
}
#endif
