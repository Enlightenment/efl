
/* blend color --> dst */

#ifdef BUILD_MMX
static void
_op_blend_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = (c >> 16) & 0xff00;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6);

   movd_m2r(c, mm5);
   MOV_RA2R(mm5, mm7)
   psllq_i2r(48, mm7);
   c |= 0xff000000;
   MOV_P2R(c, mm1, mm0)

   while (d < e)
     {
	DATA32  a = _evas_pow_lut[ca + (*d >> 24)] << 24;
	MOV_MA2R(a, mm3)
	psrlq_i2r(16, mm3);
	por_r2r(mm7, mm3);
	psrlw_i2r(1, mm3);

	MOV_P2R(*d, mm2, mm0)
	movq_r2r(mm1, mm4);

	BLEND_R2R(mm3, mm4, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	d++;
     }
}

static void
_op_blend_c_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6);

   movd_m2r(c, mm1);
   MOV_RA2R(mm1, mm3)
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);	
   punpcklbw_r2r(mm0, mm1);

   while (d < e)
     {
	MOV_P2R(*d, mm2, mm0)
	movq_r2r(mm1, mm4);

	BLEND_R2R(mm3, mm4, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	d++;
     }
}

#define _op_blend_caa_dp_mmx _op_blend_c_dp_mmx

#define _op_blend_caa_dpan_mmx _op_blend_c_dpan_mmx

#define _op_blend_c_dpas_mmx _op_blend_c_dp_mmx
#define _op_blend_caa_dpas_mmx _op_blend_c_dp_mmx

static void
init_blend_color_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_c_dp_mmx;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_caa_dp_mmx;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_c_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_caa_dpan_mmx;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_c_dpas_mmx;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)
	s = c & 0xff000000;
	MOV_MA2R(s, mm2)
	psllq_i2r(48, mm2);

	s = _evas_pow_lut[(s >> 16) + (*d >> 24)] << 24;
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

static void
_op_blend_pt_c_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6);

	movd_m2r(c, mm1);
	MOV_RA2R(mm1, mm3)
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);
	
	punpcklbw_r2r(mm0, mm1);
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

#define _op_blend_pt_caa_dp_mmx _op_blend_pt_c_dp_mmx

#define _op_blend_pt_caa_dpan_mmx _op_blend_pt_c_dpan_mmx

#define _op_blend_pt_c_dpas_mmx _op_blend_pt_c_dp_mmx
#define _op_blend_pt_caa_dpas_mmx _op_blend_pt_c_dp_mmx

static void
init_blend_color_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_pt_c_dp_mmx;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_pt_caa_dp_mmx;

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_pt_c_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_pt_caa_dpan_mmx;

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_pt_c_dpas_mmx;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_pt_caa_dpas_mmx;
}
#endif
/*-----*/

/* blend_rel color -> dst */

#ifdef BUILD_MMX
#define _op_blend_rel_c_dp_mmx _op_blend_c_dpan_mmx
#define _op_blend_rel_caa_dp_mmx _op_blend_caa_dpan_mmx

#define _op_blend_rel_c_dpan_mmx _op_blend_c_dpan_mmx
#define _op_blend_rel_caa_dpan_mmx _op_blend_caa_dpan_mmx

#define _op_blend_rel_c_dpas_mmx _op_blend_rel_c_dp_mmx
#define _op_blend_rel_caa_dpas_mmx _op_blend_rel_caa_dp_mmx

static void
init_blend_rel_color_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_c_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_caa_dp_mmx;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_c_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_caa_dpan_mmx;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_c_dpas_mmx;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
#define _op_blend_rel_pt_c_dp_mmx _op_blend_pt_c_dpan_mmx
#define _op_blend_rel_pt_caa_dp_mmx _op_blend_pt_caa_dpan_mmx

#define _op_blend_rel_pt_c_dpan_mmx _op_blend_pt_c_dpan_mmx
#define _op_blend_rel_pt_caa_dpan_mmx _op_blend_pt_caa_dpan_mmx

#define _op_blend_rel_pt_c_dpas_mmx _op_blend_rel_pt_c_dp_mmx
#define _op_blend_rel_pt_caa_dpas_mmx _op_blend_rel_pt_caa_dp_mmx

static void
init_blend_rel_color_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_pt_c_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_pt_caa_dp_mmx;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_pt_c_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pt_caa_dpan_mmx;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_pt_c_dpas_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_pt_caa_dpas_mmx;
}
#endif
