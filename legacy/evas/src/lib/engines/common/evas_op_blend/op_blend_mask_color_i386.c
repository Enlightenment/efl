
/* blend mask x color -> dst */

#ifdef BUILD_MMX
static void
_op_blend_mas_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_MA2R(c, mm7)
   psllq_i2r(48, mm7);
   l = 1 + (c >> 24);
   MOV_A2R(l, mm4)

   c |= 0xff000000;
   MOV_P2R(c, mm1, mm0)
   c = (l - 1) << 8;
   while (d < e)
     {
	DATA32 a = *m;

	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		a = _evas_pow_lut[c + (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);
		por_r2r(mm7, mm3);

		psrlw_i2r(1, mm3);

		MOV_P2R(*d, mm2, mm0)
		movq_r2r(mm1, mm5);

		BLEND_R2R(mm3, mm5, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	    default:
		MOV_A2R(a, mm3)
		MUL4_256_R2R(mm4, mm3)
		MOV_R2P(mm3, a, mm0)
		MOV_RA2R(mm3, mm2)
		psllq_i2r(48, mm2);

		a = _evas_pow_lut[(a & 0xff00) | (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);

		por_r2r(mm2, mm3);
		psrlw_i2r(1, mm3);

		MOV_P2R(*d, mm2, mm0)
		movq_r2r(mm1, mm5);

		BLEND_R2R(mm3, mm5, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

// this is broken for destination alpha - destinationa alpha is hosed.
static void
_op_blend_mas_can_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(c, mm1, mm0)
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = c;
		break;
	    default:
		MOV_MA2R(a, mm2)
		psllq_i2r(48, mm2);

		a = _evas_pow_lut[(a << 8) | (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);

		por_r2r(mm2, mm3);
		psrlw_i2r(1, mm3);

		MOV_P2R(*d, mm2, mm0)
		movq_r2r(mm1, mm5);

		BLEND_R2R(mm3, mm5, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

static void
_op_blend_mas_c_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6);

   movd_m2r(c, mm1);
   MOV_RA2R(mm1, mm3)
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);	
   punpcklbw_r2r(mm0, mm1);

   c = 1 + (c >> 24);
   MOV_A2R(c, mm4)
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		MOV_P2R(*d, mm2, mm0)
		movq_r2r(mm1, mm5);

		BLEND_R2R(mm3, mm5, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	    default:
		MOV_A2R(a, mm7)
		MUL4_256_R2R(mm4, mm7)
		packuswb_r2r(mm0, mm7);
		punpcklbw_r2r(mm7, mm7);
		psrlw_i2r(1, mm7);
		psrlq_i2r(16, mm7);

		MOV_P2R(*d, mm2, mm0)
		movq_r2r(mm1, mm5);

		BLEND_R2R(mm7, mm5, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

static void
_op_blend_mas_can_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6);
   MOV_P2R(c, mm1, mm0)
   while (d < e)
     {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		*d = c;
		break;
	    default:
		a <<= 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);
		psrlw_i2r(1, mm3);

		MOV_P2R(*d, mm2, mm0)
		movq_r2r(mm1, mm5);

		BLEND_R2R(mm3, mm5, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

#define _op_blend_mas_cn_dp_mmx _op_blend_mas_can_dp_mmx
#define _op_blend_mas_caa_dp_mmx _op_blend_mas_c_dp_mmx

#define _op_blend_mas_cn_dpan_mmx _op_blend_mas_can_dpan_mmx
#define _op_blend_mas_caa_dpan_mmx _op_blend_mas_c_dpan_mmx

#define _op_blend_mas_c_dpas_mmx _op_blend_mas_c_dp_mmx
#define _op_blend_mas_cn_dpas_mmx _op_blend_mas_cn_dp_mmx
#define _op_blend_mas_can_dpas_mmx _op_blend_mas_can_dp_mmx
#define _op_blend_mas_caa_dpas_mmx _op_blend_mas_caa_dp_mmx

static void
init_blend_mask_color_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_mas_c_dp_mmx;
// FIXME: broken   
//   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_mas_cn_dp_mmx;
//   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_mas_can_dp_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_mas_caa_dp_mmx;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_mas_c_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_mas_cn_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_mas_can_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_mas_caa_dpan_mmx;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_MMX] = _op_blend_mas_c_dpas_mmx;
// FIXME: broken   
//   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_mas_cn_dpas_mmx;
//   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_MMX] = _op_blend_mas_can_dpas_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_MMX] = _op_blend_mas_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_mas_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)
	MOV_A2R(m, mm3)
	s = 1 + (c >> 24);
	MOV_A2R(s, mm4)
	MUL4_256_R2R(mm4, mm3)
	MOV_R2P(mm3, s, mm0)
	MOV_RA2R(mm3, mm2)
	psllq_i2r(48, mm2);

	s = _evas_pow_lut[(s & 0xff00) | (*d >> 24)] << 24;
	MOV_MA2R(s, mm3)
	psrlq_i2r(16, mm3);

	por_r2r(mm2, mm3);
	psrlw_i2r(1, mm3);

	c |= 0xff000000;
	MOV_P2R(c, mm1, mm0)
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

// this is broken (i think) for destination alpha - destinationa alpha is hosed.
static void
_op_blend_pt_mas_can_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)
	MOV_MA2R(m, mm2)
	psllq_i2r(48, mm2);

	s = _evas_pow_lut[(m << 8) | (*d >> 24)] << 24;
	MOV_MA2R(s, mm3)
	psrlq_i2r(16, mm3);
	por_r2r(mm2, mm3);
	psrlw_i2r(1, mm3);

	MOV_P2R(c, mm1, mm0)
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

static void
_op_blend_pt_mas_c_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6);

	MOV_P2R(c, mm1, mm0)
	c = 1 + (c >> 24);
	MOV_A2R(c, mm4)
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
_op_blend_pt_mas_can_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6);

	MOV_P2R(c, mm1, mm0)
	c = m << 24;
	MOV_MA2R(c, mm3)
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

#define _op_blend_pt_mas_cn_dp_mmx _op_blend_pt_mas_can_dp_mmx
#define _op_blend_pt_mas_caa_dp_mmx _op_blend_pt_mas_c_dp_mmx

#define _op_blend_pt_mas_cn_dpan_mmx _op_blend_pt_mas_can_dpan_mmx
#define _op_blend_pt_mas_caa_dpan_mmx _op_blend_pt_mas_c_dpan_mmx

#define _op_blend_pt_mas_c_dpas_mmx _op_blend_pt_mas_c_dp_mmx
#define _op_blend_pt_mas_cn_dpas_mmx _op_blend_pt_mas_cn_dp_mmx
#define _op_blend_pt_mas_can_dpas_mmx _op_blend_pt_mas_can_dp_mmx
#define _op_blend_pt_mas_caa_dpas_mmx _op_blend_pt_mas_caa_dp_mmx

static void
init_blend_mask_color_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_pt_mas_c_dp_mmx;
// FIXME: broken   
//   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_mas_cn_dp_mmx;
//   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_pt_mas_can_dp_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_pt_mas_caa_dp_mmx;

   op_blend_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_pt_mas_c_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_mas_cn_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_pt_mas_can_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_pt_mas_caa_dpan_mmx;

   op_blend_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_MMX] = _op_blend_pt_mas_c_dpas_mmx;
// FIXME: broken   
//   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_pt_mas_cn_dpas_mmx;
//   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_MMX] = _op_blend_pt_mas_can_dpas_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_MMX] = _op_blend_pt_mas_caa_dpas_mmx;
}
#endif

/*-----*/

/* blend_rel mask x color -> dst */

#ifdef BUILD_MMX
#define _op_blend_rel_mas_c_dp_mmx _op_blend_mas_c_dpan_mmx
#define _op_blend_rel_mas_cn_dp_mmx _op_blend_mas_cn_dpan_mmx
#define _op_blend_rel_mas_can_dp_mmx _op_blend_mas_can_dpan_mmx
#define _op_blend_rel_mas_caa_dp_mmx _op_blend_mas_caa_dpan_mmx

#define _op_blend_rel_mas_c_dpan_mmx _op_blend_mas_c_dpan_mmx
#define _op_blend_rel_mas_cn_dpan_mmx _op_blend_mas_cn_dpan_mmx
#define _op_blend_rel_mas_can_dpan_mmx _op_blend_mas_can_dpan_mmx
#define _op_blend_rel_mas_caa_dpan_mmx _op_blend_mas_caa_dpan_mmx

#define _op_blend_rel_mas_c_dpas_mmx _op_blend_rel_mas_c_dp_mmx
#define _op_blend_rel_mas_cn_dpas_mmx _op_blend_rel_mas_cn_dp_mmx
#define _op_blend_rel_mas_can_dpas_mmx _op_blend_rel_mas_can_dp_mmx
#define _op_blend_rel_mas_caa_dpas_mmx _op_blend_rel_mas_caa_dp_mmx

static void
init_blend_rel_mask_color_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_rel_mas_c_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_mas_cn_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_rel_mas_can_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_rel_mas_caa_dp_mmx;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_rel_mas_c_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_mas_cn_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_mas_can_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_mas_caa_dpan_mmx;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AS][CPU_MMX] = _op_blend_rel_mas_c_dpas_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_mas_cn_dpas_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_mas_can_dpas_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_mas_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
#define _op_blend_rel_pt_mas_c_dp_mmx _op_blend_pt_mas_c_dpan_mmx
#define _op_blend_rel_pt_mas_cn_dp_mmx _op_blend_pt_mas_cn_dpan_mmx
#define _op_blend_rel_pt_mas_can_dp_mmx _op_blend_pt_mas_can_dpan_mmx
#define _op_blend_rel_pt_mas_caa_dp_mmx _op_blend_pt_mas_caa_dpan_mmx

#define _op_blend_rel_pt_mas_c_dpan_mmx _op_blend_pt_mas_c_dpan_mmx
#define _op_blend_rel_pt_mas_cn_dpan_mmx _op_blend_pt_mas_cn_dpan_mmx
#define _op_blend_rel_pt_mas_can_dpan_mmx _op_blend_pt_mas_can_dpan_mmx
#define _op_blend_rel_pt_mas_caa_dpan_mmx _op_blend_pt_mas_caa_dpan_mmx

#define _op_blend_rel_pt_mas_c_dpas_mmx _op_blend_rel_pt_mas_c_dp_mmx
#define _op_blend_rel_pt_mas_cn_dpas_mmx _op_blend_rel_pt_mas_cn_dp_mmx
#define _op_blend_rel_pt_mas_can_dpas_mmx _op_blend_rel_pt_mas_can_dp_mmx
#define _op_blend_rel_pt_mas_caa_dpas_mmx _op_blend_rel_pt_mas_caa_dp_mmx

static void
init_blend_rel_mask_color_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_rel_pt_mas_c_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_mas_cn_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_rel_pt_mas_can_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_rel_pt_mas_caa_dp_mmx;

   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_c_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_cn_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_can_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_caa_dpan_mmx;

   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP_AS][CPU_MMX] = _op_blend_rel_pt_mas_c_dpas_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pt_mas_cn_dpas_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_pt_mas_can_dpas_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_pt_mas_caa_dpas_mmx;
}
#endif
