/* copy color --> dst */

#ifdef BUILD_NEON

#ifndef BUILD_NEON_INTRINSICS
extern void
pixman_composite_src_n_8888_asm_neon (int32_t   w,
                                      int32_t   h,
                                      uint32_t *dst
                                      int32_t   dst_stride,
                                      uint32_t  src);
#endif

static void
_op_copy_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
#ifdef BUILD_NEON_INTRINSICS
   DATA32 *e;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = c;
                        d++;
                     });
#else
   pixman_composite_src_n_8888_asm_neon(l,1,d,l,c);
#endif
}

#define _op_copy_cn_dp_neon _op_copy_c_dp_neon
#define _op_copy_can_dp_neon _op_copy_c_dp_neon
#define _op_copy_caa_dp_neon _op_copy_c_dp_neon

#define _op_copy_cn_dpan_neon _op_copy_c_dp_neon
#define _op_copy_c_dpan_neon _op_copy_c_dp_neon
#define _op_copy_can_dpan_neon _op_copy_c_dp_neon
#define _op_copy_caa_dpan_neon _op_copy_c_dp_neon

static void
init_copy_color_span_funcs_neon(void)
{
   op_copy_span_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_cn_dp_neon;
   op_copy_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_c_dp_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_can_dp_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_caa_dp_neon;

   op_copy_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_cn_dpan_neon;
   op_copy_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_c_dpan_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_can_dpan_neon;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_c_dp_neon(DATA32 s EINA_UNUSED, DATA8 m EINA_UNUSED, DATA32 c, DATA32 *d) {
   *d = c;
}

#define _op_copy_pt_cn_dp_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_can_dp_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_caa_dp_neon _op_copy_pt_c_dp_neon

#define _op_copy_pt_cn_dpan_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_c_dpan_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_can_dpan_neon _op_copy_pt_c_dp_neon
#define _op_copy_pt_caa_dpan_neon _op_copy_pt_c_dp_neon

static void
init_copy_color_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_cn_dp_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_pt_c_dp_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_pt_can_dp_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_pt_caa_dp_neon;

   op_copy_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_cn_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_pt_c_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_pt_can_dpan_neon;
   op_copy_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_pt_caa_dpan_neon;
}
#endif

/*-----*/

/* copy_rel color --> dst */

#ifdef BUILD_NEON
static void
_op_copy_rel_c_dp_neon(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e = d + l;
   for (; d < e; d++) {
	*d = MUL_SYM(*d >> 24, c);
   }
}

#define _op_copy_rel_cn_dp_neon _op_copy_rel_c_dp_neon
#define _op_copy_rel_can_dp_neon _op_copy_rel_c_dp_neon
#define _op_copy_rel_caa_dp_neon _op_copy_rel_c_dp_neon

#define _op_copy_rel_cn_dpan_neon _op_copy_cn_dpan_neon
#define _op_copy_rel_c_dpan_neon _op_copy_c_dpan_neon
#define _op_copy_rel_can_dpan_neon _op_copy_can_dpan_neon
#define _op_copy_rel_caa_dpan_neon _op_copy_caa_dpan_neon

static void
init_copy_rel_color_span_funcs_neon(void)
{
   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_cn_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_c_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_can_dp_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_caa_dp_neon;

   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_cn_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_c_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_can_dpan_neon;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_rel_pt_c_dp_neon(DATA32 s, DATA8 m EINA_UNUSED, DATA32 c, DATA32 *d) {
   s = 1 + (*d >> 24);
   *d = MUL_256(s, c);
}


#define _op_copy_rel_pt_cn_dp_neon _op_copy_rel_pt_c_dp_neon
#define _op_copy_rel_pt_can_dp_neon _op_copy_rel_pt_c_dp_neon
#define _op_copy_rel_pt_caa_dp_neon _op_copy_rel_pt_c_dp_neon

#define _op_copy_rel_pt_cn_dpan_neon _op_copy_pt_cn_dpan_neon
#define _op_copy_rel_pt_c_dpan_neon _op_copy_pt_c_dpan_neon
#define _op_copy_rel_pt_can_dpan_neon _op_copy_pt_can_dpan_neon
#define _op_copy_rel_pt_caa_dpan_neon _op_copy_pt_caa_dpan_neon

static void
init_copy_rel_color_pt_funcs_neon(void)
{
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_cn_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_pt_c_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_pt_can_dp_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_pt_caa_dp_neon;

   op_copy_rel_pt_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_cn_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_pt_c_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pt_can_dpan_neon;
   op_copy_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pt_caa_dpan_neon;
}
#endif
