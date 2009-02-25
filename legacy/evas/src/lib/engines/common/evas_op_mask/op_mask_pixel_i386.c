
/* mask pixel --> dst */

#ifdef BUILD_MMX
static void
_op_mask_p_dp_mmx(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e = d + l;
   MOV_A2R(ALPHA_255, mm5)
   pxor_r2r(mm0, mm0);
   for (; d < e; d++) {
	MOV_P2R(*d, mm1, mm0)
	MOV_PA2R(*s, mm2)
	MUL4_SYM_R2R(mm2, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
   }
}

#define _op_mask_pas_dp_mmx _op_mask_p_dp_mmx

#define _op_mask_p_dpan_mmx _op_mask_p_dp_mmx
#define _op_mask_pas_dpan_mmx _op_mask_pas_dp_mmx

static void
init_mask_pixel_span_funcs_mmx(void)
{
   op_mask_span_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_mask_p_dp_mmx;
   op_mask_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_mask_pas_dp_mmx;

   op_mask_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mask_p_dpan_mmx;
   op_mask_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mask_pas_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_mask_pt_p_dp_mmx(DATA32 s, DATA8 m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d) {
	MOV_A2R(ALPHA_255, mm5)
	pxor_r2r(mm0, mm0);
	MOV_P2R(*d, mm1, mm0)
	MOV_PA2R(s, mm2)
	MUL4_SYM_R2R(mm2, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
}

#define _op_mask_pt_pas_dp_mmx _op_mask_pt_p_dp_mmx

#define _op_mask_pt_p_dpan_mmx _op_mask_pt_p_dp_mmx
#define _op_mask_pt_pas_dpan_mmx _op_mask_pt_pas_dp_mmx

static void
init_mask_pixel_pt_funcs_mmx(void)
{
   op_mask_pt_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_mask_pt_p_dp_mmx;
   op_mask_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_mask_pt_pas_dp_mmx;

   op_mask_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mask_pt_p_dpan_mmx;
   op_mask_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_mask_pt_pas_dpan_mmx;
}
#endif
