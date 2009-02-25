
/* copy color --> dst */

#ifdef BUILD_MMX
static void
_op_copy_c_dp_mmx(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l - 1;
   movd_m2r(c, mm1);
   movq_r2r(mm1, mm2);
   psllq_i2r(32, mm1);
   por_r2r(mm2, mm1);
   for (; d < e; d+=2) {
      movq_r2m(mm1, d[0]);
   }
   e+=1;
   for (; d < e; d++) {
      *d = c;
   }
}

#define _op_copy_cn_dp_mmx _op_copy_c_dp_mmx
#define _op_copy_can_dp_mmx _op_copy_c_dp_mmx
#define _op_copy_caa_dp_mmx _op_copy_c_dp_mmx

#define _op_copy_cn_dpan_mmx _op_copy_c_dp_mmx
#define _op_copy_c_dpan_mmx _op_copy_c_dp_mmx
#define _op_copy_can_dpan_mmx _op_copy_c_dp_mmx
#define _op_copy_caa_dpan_mmx _op_copy_c_dp_mmx

static void
init_copy_color_span_funcs_mmx(void)
{
   op_copy_span_funcs[SP_N][SM_N][SC_N][DP][CPU_MMX] = _op_copy_cn_dp_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_copy_c_dp_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_MMX] = _op_copy_can_dp_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_copy_caa_dp_mmx;

   op_copy_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_cn_dpan_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_copy_c_dpan_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_copy_can_dpan_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_copy_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_copy_pt_c_dp_mmx(DATA32 s __UNUSED__, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
     *d = c;
}

#define _op_copy_pt_cn_dp_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_can_dp_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_caa_dp_mmx _op_copy_pt_c_dp_mmx

#define _op_copy_pt_cn_dpan_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_c_dpan_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_can_dpan_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_caa_dpan_mmx _op_copy_pt_c_dp_mmx

static void
init_copy_color_pt_funcs_mmx(void)
{
   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_MMX] = _op_copy_pt_cn_dp_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_copy_pt_c_dp_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_MMX] = _op_copy_pt_can_dp_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_copy_pt_caa_dp_mmx;

   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_pt_cn_dpan_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_copy_pt_c_dpan_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_copy_pt_can_dpan_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_copy_pt_caa_dpan_mmx;
}
#endif

/*-----*/

/* copy_rel color --> dst */

#ifdef BUILD_MMX
static void
_op_copy_rel_c_dp_mmx(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   for (; d < e; d++) {
	DATA32  da = 1 + (*d >> 24);
	MOV_A2R(da, mm1)
	MUL4_256_R2R(mm2, mm1)
	MOV_R2P(mm1, *d, mm0)
   }
}

#define _op_copy_rel_cn_dp_mmx _op_copy_rel_c_dp_mmx
#define _op_copy_rel_can_dp_mmx _op_copy_rel_c_dp_mmx
#define _op_copy_rel_caa_dp_mmx _op_copy_rel_c_dp_mmx

#define _op_copy_rel_cn_dpan_mmx _op_copy_cn_dpan_mmx
#define _op_copy_rel_c_dpan_mmx _op_copy_c_dpan_mmx
#define _op_copy_rel_can_dpan_mmx _op_copy_can_dpan_mmx
#define _op_copy_rel_caa_dpan_mmx _op_copy_caa_dpan_mmx

static void
init_copy_rel_color_span_funcs_mmx(void)
{
   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_cn_dp_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_copy_rel_c_dp_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_MMX] = _op_copy_rel_can_dp_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_copy_rel_caa_dp_mmx;

   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_cn_dpan_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_copy_rel_c_dpan_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_copy_rel_can_dpan_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_copy_rel_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_copy_rel_pt_c_dp_mmx(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
	s = 1 + (*d >> 24);
	pxor_r2r(mm0, mm0);
	MOV_P2R(c, mm2, mm0)
	MOV_A2R(s, mm1)
	MUL4_256_R2R(mm2, mm1)
	MOV_R2P(mm1, *d, mm0)
}


#define _op_copy_rel_pt_cn_dp_mmx _op_copy_rel_pt_c_dp_mmx
#define _op_copy_rel_pt_can_dp_mmx _op_copy_rel_pt_c_dp_mmx
#define _op_copy_rel_pt_caa_dp_mmx _op_copy_rel_pt_c_dp_mmx

#define _op_copy_rel_pt_cn_dpan_mmx _op_copy_pt_cn_dpan_mmx
#define _op_copy_rel_pt_c_dpan_mmx _op_copy_pt_c_dpan_mmx
#define _op_copy_rel_pt_can_dpan_mmx _op_copy_pt_can_dpan_mmx
#define _op_copy_rel_pt_caa_dpan_mmx _op_copy_pt_caa_dpan_mmx

static void
init_copy_rel_color_pt_funcs_mmx(void)
{
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_pt_cn_dp_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_MMX] = _op_copy_rel_pt_c_dp_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_MMX] = _op_copy_rel_pt_can_dp_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_MMX] = _op_copy_rel_pt_caa_dp_mmx;

   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pt_cn_dpan_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_MMX] = _op_copy_rel_pt_c_dpan_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_copy_rel_pt_can_dpan_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_copy_rel_pt_caa_dpan_mmx;
}
#endif
