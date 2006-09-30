
/* mul pixel x color --> dst */

#ifdef BUILD_MMX
static void
_op_mul_p_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   MOV_A2R(ALPHA_255, mm5)
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   while (d < e)
     {
	MOV_P2R(*s, mm3, mm0)
	MUL4_SYM_R2R(mm2, mm3, mm5)
	MOV_P2R(*d, mm1, mm0)
	MUL4_SYM_R2R(mm3, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
	s++;  d++;
     }
}

#define _op_mul_pas_c_dp_mmx _op_mul_p_c_dp_mmx
#define _op_mul_pan_c_dp_mmx _op_mul_p_c_dp_mmx
#define _op_mul_p_can_dp_mmx _op_mul_p_c_dp_mmx
#define _op_mul_pas_can_dp_mmx _op_mul_p_c_dp_mmx
#define _op_mul_pan_can_dp_mmx _op_mul_p_c_dp_mmx
#define _op_mul_p_caa_dp_mmx _op_mul_p_c_dp_mmx
#define _op_mul_pas_caa_dp_mmx _op_mul_p_c_dp_mmx
#define _op_mul_pan_caa_dp_mmx _op_mul_p_c_dp_mmx

#define _op_mul_p_c_dpan_mmx _op_mul_p_c_dp_mmx
#define _op_mul_pan_c_dpan_mmx _op_mul_pan_c_dp_mmx
#define _op_mul_pas_c_dpan_mmx _op_mul_pas_c_dp_mmx
#define _op_mul_p_can_dpan_mmx _op_mul_p_can_dp_mmx
#define _op_mul_pan_can_dpan_mmx _op_mul_pan_can_dp_mmx
#define _op_mul_pas_can_dpan_mmx _op_mul_pas_can_dp_mmx
#define _op_mul_p_caa_dpan_mmx _op_mul_p_caa_dp_mmx
#define _op_mul_pan_caa_dpan_mmx _op_mul_pan_caa_dp_mmx
#define _op_mul_pas_caa_dpan_mmx _op_mul_pas_caa_dp_mmx

static void
init_mul_pixel_color_span_funcs_mmx(void)
{
   op_mul_span_funcs[SP][SM_N][SC][DP][CPU_MMX] = _op_mul_p_c_dp_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC][DP][CPU_MMX] = _op_mul_pas_c_dp_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC][DP][CPU_MMX] = _op_mul_pan_c_dp_mmx;
   op_mul_span_funcs[SP][SM_N][SC_AN][DP][CPU_MMX] = _op_mul_p_can_dp_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_MMX] = _op_mul_pas_can_dp_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_MMX] = _op_mul_pan_can_dp_mmx;
   op_mul_span_funcs[SP][SM_N][SC_AA][DP][CPU_MMX] = _op_mul_p_caa_dp_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_MMX] = _op_mul_pas_caa_dp_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_MMX] = _op_mul_pan_caa_dp_mmx;

   op_mul_span_funcs[SP][SM_N][SC][DP_AN][CPU_MMX] = _op_mul_p_c_dpan_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_MMX] = _op_mul_pas_c_dpan_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_MMX] = _op_mul_pan_c_dpan_mmx;
   op_mul_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_mul_p_can_dpan_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_mul_pas_can_dpan_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_mul_pan_can_dpan_mmx;
   op_mul_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mul_p_caa_dpan_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mul_pas_caa_dpan_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mul_pan_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_mul_pt_p_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	MOV_A2R(ALPHA_255, mm5)
	pxor_r2r(mm0, mm0);
	MOV_P2R(c, mm2, mm0)
	MOV_P2R(s, mm3, mm0)
	MUL4_SYM_R2R(mm2, mm3, mm5)
	MOV_P2R(*d, mm1, mm0)
	MUL4_SYM_R2R(mm3, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
}

#define _op_mul_pt_pas_c_dp_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_pan_c_dp_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_p_can_dp_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_pas_can_dp_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_pan_can_dp_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_p_caa_dp_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_pas_caa_dp_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_pan_caa_dp_mmx _op_mul_pt_p_c_dp_mmx

#define _op_mul_pt_p_c_dpan_mmx _op_mul_pt_p_c_dp_mmx
#define _op_mul_pt_pan_c_dpan_mmx _op_mul_pt_pan_c_dp_mmx
#define _op_mul_pt_pas_c_dpan_mmx _op_mul_pt_pas_c_dp_mmx
#define _op_mul_pt_p_can_dpan_mmx _op_mul_pt_p_can_dp_mmx
#define _op_mul_pt_pan_can_dpan_mmx _op_mul_pt_pan_can_dp_mmx
#define _op_mul_pt_pas_can_dpan_mmx _op_mul_pt_pas_can_dp_mmx
#define _op_mul_pt_p_caa_dpan_mmx _op_mul_pt_p_caa_dp_mmx
#define _op_mul_pt_pan_caa_dpan_mmx _op_mul_pt_pan_caa_dp_mmx
#define _op_mul_pt_pas_caa_dpan_mmx _op_mul_pt_pas_caa_dp_mmx

static void
init_mul_pixel_color_pt_funcs_mmx(void)
{
   op_mul_pt_funcs[SP][SM_N][SC][DP][CPU_MMX] = _op_mul_pt_p_c_dp_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC][DP][CPU_MMX] = _op_mul_pt_pas_c_dp_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC][DP][CPU_MMX] = _op_mul_pt_pan_c_dp_mmx;
   op_mul_pt_funcs[SP][SM_N][SC_AN][DP][CPU_MMX] = _op_mul_pt_p_can_dp_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_MMX] = _op_mul_pt_pas_can_dp_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_MMX] = _op_mul_pt_pan_can_dp_mmx;
   op_mul_pt_funcs[SP][SM_N][SC_AA][DP][CPU_MMX] = _op_mul_pt_p_caa_dp_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_MMX] = _op_mul_pt_pas_caa_dp_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_MMX] = _op_mul_pt_pan_caa_dp_mmx;

   op_mul_pt_funcs[SP][SM_N][SC][DP_AN][CPU_MMX] = _op_mul_pt_p_c_dpan_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_MMX] = _op_mul_pt_pas_c_dpan_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_MMX] = _op_mul_pt_pan_c_dpan_mmx;
   op_mul_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_mul_pt_p_can_dpan_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_mul_pt_pas_can_dpan_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_mul_pt_pan_can_dpan_mmx;
   op_mul_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mul_pt_p_caa_dpan_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mul_pt_pas_caa_dpan_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mul_pt_pan_caa_dpan_mmx;
}
#endif
