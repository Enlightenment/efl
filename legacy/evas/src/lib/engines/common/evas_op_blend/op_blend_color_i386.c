
/* blend color --> dst */

#ifdef BUILD_MMX
static void
_op_blend_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   c = 256 - (c >> 24);
   MOV_A2R(c, mm3)
   while (d < e) {
	MOV_P2R(*d, mm1, mm0)
	MUL4_256_R2R(mm3, mm1)
	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
	d++;
     }
}

#define _op_blend_caa_dp_mmx _op_blend_c_dp_mmx

#define _op_blend_c_dpan_mmx _op_blend_c_dp_mmx
#define _op_blend_caa_dpan_mmx _op_blend_c_dpan_mmx

static void
init_blend_color_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_c_dp_mmx;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_caa_dp_mmx;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_c_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_P2R(c, mm2, mm0)
	c = 256 - (c >> 24);
	MOV_A2R(c, mm3)
	MOV_P2R(*d, mm1, mm0)
	MUL4_256_R2R(mm3, mm1)
	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
}

#define _op_blend_pt_caa_dp_mmx _op_blend_pt_c_dp_mmx

#define _op_blend_pt_c_dpan_mmx _op_blend_pt_c_dp_mmx
#define _op_blend_pt_caa_dpan_mmx _op_blend_pt_c_dpan_mmx

static void
init_blend_color_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_pt_c_dp_mmx;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_pt_caa_dp_mmx;

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_pt_c_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_pt_caa_dpan_mmx;
}
#endif
/*-----*/

/* blend_rel color -> dst */

#ifdef BUILD_MMX
static void
_op_blend_rel_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   c = 256 - (c >> 24);
   MOV_A2R(c, mm3)
   MOV_A2R(ALPHA_255, mm5)
   while (d < e) {
	MOV_P2R(*d, mm1, mm0)
	MOV_RA2R(mm1, mm4)
	MUL4_256_R2R(mm3, mm1)
	MUL4_SYM_R2R(mm2, mm4, mm5)
	paddw_r2r(mm4, mm1);
	MOV_R2P(mm1, *d, mm0)
	d++;
     }
}

#define _op_blend_rel_caa_dp_mmx _op_blend_rel_c_dp_mmx

#define _op_blend_rel_c_dpan_mmx _op_blend_c_dpan_mmx
#define _op_blend_rel_caa_dpan_mmx _op_blend_caa_dpan_mmx

static void
init_blend_rel_color_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_c_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_caa_dp_mmx;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_c_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_rel_pt_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_256, mm6)
	MOV_A2R(ALPHA_255, mm5)

	MOV_P2R(c, mm2, mm0)
	MOV_RA2R(mm2, mm1)
	psubw_r2r(mm1, mm6);

	MOV_P2R(*d, mm1, mm0)
	MOV_RA2R(mm1, mm4)
	MUL4_256_R2R(mm6, mm1)

	MUL4_SYM_R2R(mm4, mm2, mm5)
	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
}

#define _op_blend_rel_pt_caa_dp_mmx _op_blend_rel_pt_c_dp_mmx

#define _op_blend_rel_pt_c_dpan_mmx _op_blend_pt_c_dpan_mmx
#define _op_blend_rel_pt_caa_dpan_mmx _op_blend_pt_caa_dpan_mmx

static void
init_blend_rel_color_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_pt_c_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_pt_caa_dp_mmx;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_pt_c_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pt_caa_dpan_mmx;
}
#endif
