
/* blend pixel x color --> dst */

#ifdef BUILD_NEON
static void
_op_blend_p_c_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        DATA32 sc = MUL4_SYM(c, *s);
                        alpha = 256 - (sc >> 24);
                        *d = sc + MUL_256(alpha, *d);
                        d++;
                        s++;
                     });
}

static void
_op_blend_pan_can_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d++ = 0xff000000 + MUL3_SYM(c, *s);
                        s++;
                     });
}

static void
_op_blend_pan_caa_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   c = 1 + (c & 0xff);
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = INTERP_256(c, *s, *d);
                        d++;
                        s++;
                     });
}

#define _op_blend_pas_c_dp_neon _op_blend_p_c_dp_neon
#define _op_blend_pan_c_dp_neon _op_blend_p_c_dp_neon
#define _op_blend_p_can_dp_neon _op_blend_p_c_dp_neon
#define _op_blend_pas_can_dp_neon _op_blend_p_c_dp_neon
#define _op_blend_p_caa_dp_neon _op_blend_p_c_dp_neon
#define _op_blend_pas_caa_dp_neon _op_blend_p_c_dp_neon

#define _op_blend_p_c_dpan_neon _op_blend_p_c_dp_neon
#define _op_blend_pas_c_dpan_neon _op_blend_pas_c_dp_neon
#define _op_blend_pan_c_dpan_neon _op_blend_pan_c_dp_neon
#define _op_blend_p_can_dpan_neon _op_blend_p_can_dp_neon
#define _op_blend_pas_can_dpan_neon _op_blend_pas_can_dp_neon
#define _op_blend_pan_can_dpan_neon _op_blend_pan_can_dp_neon
#define _op_blend_p_caa_dpan_neon _op_blend_p_caa_dp_neon
#define _op_blend_pas_caa_dpan_neon _op_blend_pas_caa_dp_neon
#define _op_blend_pan_caa_dpan_neon _op_blend_pan_caa_dp_neon


static void
init_blend_pixel_color_span_funcs_neon(void)
{
   op_blend_span_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_blend_p_c_dp_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_blend_pas_c_dp_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_blend_pan_c_dp_neon;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_p_can_dp_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pas_can_dp_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pan_can_dp_neon;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_p_caa_dp_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pas_caa_dp_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pan_caa_dp_neon;

   op_blend_span_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_p_c_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pas_c_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pan_c_dpan_neon;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_p_can_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pas_can_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pan_can_dpan_neon;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_p_caa_dpan_neon;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pas_caa_dpan_neon;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pan_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_blend_pt_p_c_dp_neon(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   s = MUL4_SYM(c, s);
   c = 256 - (s >> 24);
   *d = s + MUL_256(c, *d);
}

#define _op_blend_pt_pas_c_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_c_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_can_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_can_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_can_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_caa_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_caa_dp_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_caa_dp_neon _op_blend_pt_p_c_dp_neon

#define _op_blend_pt_p_c_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_c_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_c_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_can_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_can_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_can_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_p_caa_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pas_caa_dpan_neon _op_blend_pt_p_c_dp_neon
#define _op_blend_pt_pan_caa_dpan_neon _op_blend_pt_p_c_dp_neon

static void
init_blend_pixel_color_pt_funcs_neon(void)
{
   op_blend_pt_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_p_c_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_pas_c_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_blend_pt_pan_c_dp_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pt_p_can_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pt_pas_can_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_blend_pt_pan_can_dp_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_p_caa_dp_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_pas_caa_dp_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_blend_pt_pan_caa_dp_neon;

   op_blend_pt_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_p_c_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_pas_c_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_pt_pan_c_dpan_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pt_p_can_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pt_pas_can_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_pt_pan_can_dpan_neon;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_p_caa_dpan_neon;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_pas_caa_dpan_neon;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_pt_pan_caa_dpan_neon;
}
#endif

/*-----*/

/* blend_rel pixel x color -> dst */

#ifdef BUILD_NEON

#define _op_blend_rel_p_c_dpan_neon _op_blend_p_c_dpan_neon
#define _op_blend_rel_pas_c_dpan_neon _op_blend_pas_c_dpan_neon
#define _op_blend_rel_pan_c_dpan_neon _op_blend_pan_c_dpan_neon
#define _op_blend_rel_p_can_dpan_neon _op_blend_p_can_dpan_neon
#define _op_blend_rel_pas_can_dpan_neon _op_blend_pas_can_dpan_neon
#define _op_blend_rel_pan_can_dpan_neon _op_blend_pan_can_dpan_neon
#define _op_blend_rel_p_caa_dpan_neon _op_blend_p_caa_dpan_neon
#define _op_blend_rel_pas_caa_dpan_neon _op_blend_pas_caa_dpan_neon
#define _op_blend_rel_pan_caa_dpan_neon _op_blend_pan_caa_dpan_neon

static void
init_blend_rel_pixel_color_span_funcs_neon(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_p_c_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pas_c_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pan_c_dpan_neon;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_p_can_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pas_can_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pan_can_dpan_neon;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_p_caa_dpan_neon;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pas_caa_dpan_neon;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pan_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON

#define _op_blend_rel_pt_p_c_dpan_neon _op_blend_pt_p_c_dpan_neon
#define _op_blend_rel_pt_pas_c_dpan_neon _op_blend_pt_pas_c_dpan_neon
#define _op_blend_rel_pt_pan_c_dpan_neon _op_blend_pt_pan_c_dpan_neon
#define _op_blend_rel_pt_p_can_dpan_neon _op_blend_pt_p_can_dpan_neon
#define _op_blend_rel_pt_pas_can_dpan_neon _op_blend_pt_pas_can_dpan_neon
#define _op_blend_rel_pt_pan_can_dpan_neon _op_blend_pt_pan_can_dpan_neon
#define _op_blend_rel_pt_p_caa_dpan_neon _op_blend_pt_p_caa_dpan_neon
#define _op_blend_rel_pt_pas_caa_dpan_neon _op_blend_pt_pas_caa_dpan_neon
#define _op_blend_rel_pt_pan_caa_dpan_neon _op_blend_pt_pan_caa_dpan_neon

static void
init_blend_rel_pixel_color_pt_funcs_neon(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_c_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_c_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_c_dpan_neon;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_can_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_can_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_can_dpan_neon;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_p_caa_dpan_neon;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_pas_caa_dpan_neon;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_blend_rel_pt_pan_caa_dpan_neon;
}
#endif
