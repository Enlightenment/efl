
/* mul pixel --> dst */

#ifdef BUILD_MMX
static void
_op_mul_p_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = s + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   for (; s < e; s++, d++) {
	MOV_P2R(*d, mm1, mm0)
	MOV_P2R(*s, mm2, mm0)
	MUL4_SYM_R2R(mm2, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
   }
}

#define _op_mul_pas_dp_mmx _op_mul_p_dp_mmx
#define _op_mul_pan_dp_mmx _op_mul_p_dp_mmx

#define _op_mul_p_dpan_mmx _op_mul_p_dp_mmx
#define _op_mul_pan_dpan_mmx _op_mul_pan_dp_mmx
#define _op_mul_pas_dpan_mmx _op_mul_pas_dp_mmx

static void
init_mul_pixel_span_funcs_mmx(void)
{
   op_mul_span_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_mul_p_dp_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_mul_pan_dp_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_mul_pas_dp_mmx;

   op_mul_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mul_p_dpan_mmx;
   op_mul_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mul_pan_dpan_mmx;
   op_mul_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mul_pas_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_mul_pt_p_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_255, mm5)
	MOV_P2R(*d, mm1, mm0)
	MOV_P2R(s, mm2, mm0)
	MUL4_SYM_R2R(mm2, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
}

#define _op_mul_pt_pan_dp_mmx _op_mul_pt_p_dp_mmx
#define _op_mul_pt_pas_dp_mmx _op_mul_pt_p_dp_mmx

#define _op_mul_pt_p_dpan_mmx _op_mul_pt_p_dp_mmx
#define _op_mul_pt_pan_dpan_mmx _op_mul_pt_pan_dp_mmx
#define _op_mul_pt_pas_dpan_mmx _op_mul_pt_pas_dp_mmx

static void
init_mul_pixel_pt_funcs_mmx(void)
{
   op_mul_pt_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_mul_pt_p_dp_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_mul_pt_pan_dp_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_mul_pt_pas_dp_mmx;

   op_mul_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mul_pt_p_dpan_mmx;
   op_mul_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mul_pt_pan_dpan_mmx;
   op_mul_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mul_pt_pas_dpan_mmx;
}
#endif
