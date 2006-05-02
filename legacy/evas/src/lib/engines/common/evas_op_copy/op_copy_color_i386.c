
/* copy color --> dst */

#ifdef BUILD_MMX
static void
_op_copy_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
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

#define _op_copy_cn_dpas_mmx _op_copy_c_dp_mmx
#define _op_copy_c_dpas_mmx _op_copy_c_dp_mmx
#define _op_copy_can_dpas_mmx _op_copy_c_dp_mmx
#define _op_copy_caa_dpas_mmx _op_copy_c_dp_mmx

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

   op_copy_span_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_cn_dpas_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_copy_c_dpas_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_copy_can_dpas_mmx;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_copy_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_copy_pt_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
     *d = c;
}

#define _op_copy_pt_cn_dp_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_can_dp_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_caa_dp_mmx _op_copy_pt_c_dp_mmx

#define _op_copy_pt_cn_dpan_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_c_dpan_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_can_dpan_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_caa_dpan_mmx _op_copy_pt_c_dp_mmx

#define _op_copy_pt_cn_dpas_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_c_dpas_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_can_dpas_mmx _op_copy_pt_c_dp_mmx
#define _op_copy_pt_caa_dpas_mmx _op_copy_pt_c_dp_mmx

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

   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_pt_cn_dpas_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_copy_pt_c_dpas_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_copy_pt_can_dpas_mmx;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_copy_pt_caa_dpas_mmx;
}
#endif

/*-----*/

/* copy_rel color --> dst */

#ifdef BUILD_MMX
static void
_op_copy_rel_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = 1 + (c >> 24);
   c &= 0x00ffffff;
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   MOV_A2R(ca, mm3)
   psllq_i2r(48, mm3);
   for (; d < e; d++) {
	MOV_P2R(*d, mm1, mm0)
	MUL4_256_R2R(mm3, mm1)
	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
   }
}

static void
_op_copy_rel_can_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   c &= 0x00ffffff;
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   MOV_P2R(PIX_BLACK, mm6, mm0)
   for (; d < e; d++) {
	MOV_P2R(*d, mm1, mm0)
	pand_r2r(mm6, mm1);
	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
   }
}

#define _op_copy_rel_cn_dp_mmx _op_copy_rel_can_dp_mmx
#define _op_copy_rel_caa_dp_mmx _op_copy_rel_c_dp_mmx

#define _op_copy_rel_cn_dpan_mmx _op_copy_cn_dpan_mmx
#define _op_copy_rel_c_dpan_mmx _op_copy_c_dpan_mmx
#define _op_copy_rel_can_dpan_mmx _op_copy_can_dpan_mmx
#define _op_copy_rel_caa_dpan_mmx _op_copy_caa_dpan_mmx

#define _op_copy_rel_cn_dpas_mmx _op_copy_rel_cn_dp_mmx
#define _op_copy_rel_c_dpas_mmx _op_copy_rel_c_dp_mmx
#define _op_copy_rel_can_dpas_mmx _op_copy_rel_can_dp_mmx
#define _op_copy_rel_caa_dpas_mmx _op_copy_rel_caa_dp_mmx

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

   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_cn_dpas_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_copy_rel_c_dpas_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_copy_rel_can_dpas_mmx;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_copy_rel_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_copy_rel_pt_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   DATA32 ca = 1 + (c >> 24);
   c &= 0x00ffffff;
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   MOV_A2R(ca, mm3)
   psllq_i2r(48, mm3);
   MOV_P2R(*d, mm1, mm0)
   MUL4_256_R2R(mm3, mm1)
   paddw_r2r(mm2, mm1);
   MOV_R2P(mm1, *d, mm0)
}

static void
_op_copy_rel_pt_cn_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d | 0x00ffffff);
}

static void
_op_copy_rel_pt_can_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d & 0xff000000) + (c & 0x00ffffff);
}

#define _op_copy_rel_pt_caa_dp_mmx _op_copy_rel_pt_c_dp_mmx

#define _op_copy_rel_pt_cn_dpan_mmx _op_copy_pt_cn_dpan_mmx
#define _op_copy_rel_pt_c_dpan_mmx _op_copy_pt_c_dpan_mmx
#define _op_copy_rel_pt_can_dpan_mmx _op_copy_pt_can_dpan_mmx
#define _op_copy_rel_pt_caa_dpan_mmx _op_copy_pt_caa_dpan_mmx

#define _op_copy_rel_pt_cn_dpas_mmx _op_copy_rel_pt_cn_dp_mmx
#define _op_copy_rel_pt_c_dpas_mmx _op_copy_rel_pt_c_dp_mmx
#define _op_copy_rel_pt_can_dpas_mmx _op_copy_rel_pt_can_dp_mmx
#define _op_copy_rel_pt_caa_dpas_mmx _op_copy_rel_pt_caa_dp_mmx

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

   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_pt_cn_dpas_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP_AS][CPU_MMX] = _op_copy_rel_pt_c_dpas_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_copy_rel_pt_can_dpas_mmx;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_copy_rel_pt_caa_dpas_mmx;
}
#endif
