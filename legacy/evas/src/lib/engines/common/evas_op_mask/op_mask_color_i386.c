
/* mask color --> dst */

#ifdef BUILD_MMX
static void
_op_mask_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c = 1 + (c >> 24);
   MOV_A2R(c, mm2)
   pxor_r2r(mm0, mm0);
   for (; d < e; d++) {
	MOV_P2R(*d, mm1, mm0)
	MUL4_256_R2R(mm2, mm1)
	MOV_R2P(mm1, *d, mm0)
   }
}

#define _op_mask_caa_dp_mmx _op_mask_c_dp_mmx

#define _op_mask_c_dpan_mmx _op_mask_c_dp_mmx
#define _op_mask_caa_dpan_mmx _op_mask_caa_dp_mmx

static void
init_mask_color_span_funcs_mmx(void)
{
   op_mask_span_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_mask_c_dp_mmx;
   op_mask_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_mask_caa_dp_mmx;

   op_mask_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_mask_c_dpan_mmx;
   op_mask_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mask_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_mask_pt_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = 1 + (c >> 24);
	MOV_A2R(c, mm2)
	pxor_r2r(mm0, mm0);
	MOV_P2R(*d, mm1, mm0)
	MUL4_256_R2R(mm2, mm1)
	MOV_R2P(mm1, *d, mm0)
}

#define _op_mask_pt_caa_dp_mmx _op_mask_pt_c_dp_mmx

#define _op_mask_pt_c_dpan_mmx _op_mask_pt_c_dp_mmx
#define _op_mask_pt_caa_dpan_mmx _op_mask_pt_caa_dp_mmx

static void
init_mask_color_pt_funcs_mmx(void)
{
   op_mask_pt_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_mask_pt_c_dp_mmx;
   op_mask_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_mask_pt_caa_dp_mmx;

   op_mask_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_mask_pt_c_dpan_mmx;
   op_mask_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_mask_pt_caa_dpan_mmx;
}
#endif
