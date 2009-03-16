
/* copy pixel x color --> dst */

#ifdef BUILD_NEON
static void
_op_copy_p_c_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = MUL4_SYM(c, *s);
                        d++;
                        s++;
                     });
}


static void
_op_copy_p_caa_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {   // FIXME: neon-it
   // FIXME: neon-it
   DATA32 *e;
   c = 1 + (c >> 24);
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = MUL_256(c, *s);
                        d++;
                        s++;
                     });
}


#define _op_copy_pas_c_dp_neon _op_copy_p_c_dp_neon
#define _op_copy_pan_c_dp_neon _op_copy_p_c_dp_neon
#define _op_copy_p_can_dp_neon _op_copy_p_c_dp_neon
#define _op_copy_pas_can_dp_neon _op_copy_pas_c_dp_neon
#define _op_copy_pan_can_dp_neon _op_copy_pan_c_dp_neon
#define _op_copy_pas_caa_dp_neon _op_copy_p_caa_dp_neon
#define _op_copy_pan_caa_dp_neon _op_copy_p_caa_dp_neon

#define _op_copy_p_c_dpan_neon _op_copy_p_c_dp_neon
#define _op_copy_pas_c_dpan_neon _op_copy_pas_c_dp_neon
#define _op_copy_pan_c_dpan_neon _op_copy_pan_c_dp_neon
#define _op_copy_p_can_dpan_neon _op_copy_p_can_dp_neon
#define _op_copy_pas_can_dpan_neon _op_copy_pas_can_dp_neon
#define _op_copy_pan_can_dpan_neon _op_copy_pan_can_dp_neon
#define _op_copy_p_caa_dpan_neon _op_copy_p_caa_dp_neon
#define _op_copy_pas_caa_dpan_neon _op_copy_pas_caa_dp_neon
#define _op_copy_pan_caa_dpan_neon _op_copy_pan_caa_dp_neon

static void
init_copy_pixel_color_span_funcs_neon(void)
{
   op_copy_span_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_copy_p_c_dp_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_copy_pas_c_dp_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_copy_pan_c_dp_neon;
   op_copy_span_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_p_can_dp_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_pas_can_dp_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_pan_can_dp_neon;
   op_copy_span_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_p_caa_dp_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_pas_caa_dp_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_pan_caa_dp_neon;

   op_copy_span_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_p_c_dpan_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_pas_c_dpan_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_pan_c_dpan_neon;
   op_copy_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_p_can_dpan_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_pas_can_dpan_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_pan_can_dpan_neon;
   op_copy_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_p_caa_dpan_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_pas_caa_dpan_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_pan_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_p_c_dp_neon(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   *d = MUL4_SYM(c, s);
}

#define _op_copy_pt_pas_c_dp_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_pan_c_dp_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_p_can_dp_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_pas_can_dp_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_pan_can_dp_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_p_caa_dp_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_pas_caa_dp_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_pan_caa_dp_neon _op_copy_pt_p_c_dp_neon

#define _op_copy_pt_p_c_dpan_neon _op_copy_pt_p_c_dp_neon
#define _op_copy_pt_pas_c_dpan_neon _op_copy_pt_pas_c_dp_neon
#define _op_copy_pt_pan_c_dpan_neon _op_copy_pt_pan_c_dp_neon
#define _op_copy_pt_p_can_dpan_neon _op_copy_pt_p_can_dp_neon
#define _op_copy_pt_pas_can_dpan_neon _op_copy_pt_pas_can_dp_neon
#define _op_copy_pt_pan_can_dpan_neon _op_copy_pt_pan_can_dp_neon
#define _op_copy_pt_p_caa_dpan_neon _op_copy_pt_p_caa_dp_neon
#define _op_copy_pt_pas_caa_dpan_neon _op_copy_pt_pas_caa_dp_neon
#define _op_copy_pt_pan_caa_dpan_neon _op_copy_pt_pan_caa_dp_neon

static void
init_copy_pixel_color_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_copy_pt_p_c_dp_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_copy_pt_pas_c_dp_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_copy_pt_pan_c_dp_neon;
   op_copy_pt_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_pt_p_can_dp_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_pt_pas_can_dp_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_pt_pan_can_dp_neon;
   op_copy_pt_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_pt_p_caa_dp_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_pt_pas_caa_dp_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_pt_pan_caa_dp_neon;

   op_copy_pt_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_pt_p_c_dpan_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_pt_pas_c_dpan_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_pt_pan_c_dpan_neon;
   op_copy_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_pt_p_can_dpan_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_pt_pas_can_dpan_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_pt_pan_can_dpan_neon;
   op_copy_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_pt_p_caa_dpan_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_pt_pas_caa_dpan_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_pt_pan_caa_dpan_neon;
}
#endif

/*-----*/

/* copy_rel pixel x color --> dst */

#ifdef BUILD_NEON
static void
_op_copy_rel_p_c_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        DATA32 cs = MUL4_SYM(c, *s);
                        *d = MUL_SYM(*d >> 24, cs);
                        d++;
                        s++;
                     });
}


#define _op_copy_rel_pas_c_dp_neon _op_copy_rel_p_c_dp_neon
#define _op_copy_rel_pan_c_dp_neon _op_copy_rel_p_c_dp_neon
#define _op_copy_rel_p_can_dp_neon _op_copy_rel_p_c_dp_neon
#define _op_copy_rel_pas_can_dp_neon _op_copy_rel_p_c_dp_neon
#define _op_copy_rel_pan_can_dp_neon _op_copy_rel_p_c_dp_neon
#define _op_copy_rel_p_caa_dp_neon _op_copy_rel_p_c_dp_neon
#define _op_copy_rel_pas_caa_dp_neon _op_copy_rel_p_c_dp_neon
#define _op_copy_rel_pan_caa_dp_neon _op_copy_rel_p_c_dp_neon

#define _op_copy_rel_p_c_dpan_neon _op_copy_p_c_dpan_neon
#define _op_copy_rel_pas_c_dpan_neon _op_copy_pas_c_dpan_neon
#define _op_copy_rel_pan_c_dpan_neon _op_copy_pan_c_dpan_neon
#define _op_copy_rel_p_can_dpan_neon _op_copy_p_can_dpan_neon
#define _op_copy_rel_pas_can_dpan_neon _op_copy_pas_can_dpan_neon
#define _op_copy_rel_pan_can_dpan_neon _op_copy_pan_can_dpan_neon
#define _op_copy_rel_p_caa_dpan_neon _op_copy_p_caa_dpan_neon
#define _op_copy_rel_pas_caa_dpan_neon _op_copy_pas_caa_dpan_neon
#define _op_copy_rel_pan_caa_dpan_neon _op_copy_pan_caa_dpan_neon

static void
init_copy_rel_pixel_color_span_funcs_neon(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_p_c_dp_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_pas_c_dp_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_pan_c_dp_neon;
   op_copy_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_p_can_dp_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_pas_can_dp_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_pan_can_dp_neon;
   op_copy_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_p_caa_dp_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_pas_caa_dp_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_pan_caa_dp_neon;

   op_copy_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_p_c_dpan_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_pas_c_dpan_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_pan_c_dpan_neon;
   op_copy_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_p_can_dpan_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pas_can_dpan_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pan_can_dpan_neon;
   op_copy_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_p_caa_dpan_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pas_caa_dpan_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pan_caa_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_rel_pt_p_c_dp_neon(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   s = MUL4_SYM(c, s);
   *d = MUL_SYM(*d >> 24, s);
}


#define _op_copy_rel_pt_pas_c_dp_neon _op_copy_rel_pt_p_c_dp_neon
#define _op_copy_rel_pt_pan_c_dp_neon _op_copy_rel_pt_p_c_dp_neon
#define _op_copy_rel_pt_p_can_dp_neon _op_copy_rel_pt_p_c_dp_neon
#define _op_copy_rel_pt_pas_can_dp_neon _op_copy_rel_pt_p_c_dp_neon
#define _op_copy_rel_pt_pan_can_dp_neon _op_copy_rel_pt_p_c_dp_neon
#define _op_copy_rel_pt_p_caa_dp_neon _op_copy_rel_pt_p_c_dp_neon
#define _op_copy_rel_pt_pas_caa_dp_neon _op_copy_rel_pt_p_c_dp_neon
#define _op_copy_rel_pt_pan_caa_dp_neon _op_copy_rel_pt_p_c_dp_neon

#define _op_copy_rel_pt_p_c_dpan_neon _op_copy_pt_p_c_dpan_neon
#define _op_copy_rel_pt_pas_c_dpan_neon _op_copy_pt_pas_c_dpan_neon
#define _op_copy_rel_pt_pan_c_dpan_neon _op_copy_pt_pan_c_dpan_neon
#define _op_copy_rel_pt_p_can_dpan_neon _op_copy_pt_p_can_dpan_neon
#define _op_copy_rel_pt_pas_can_dpan_neon _op_copy_pt_pas_can_dpan_neon
#define _op_copy_rel_pt_pan_can_dpan_neon _op_copy_pt_pan_can_dpan_neon
#define _op_copy_rel_pt_p_caa_dpan_neon _op_copy_pt_p_caa_dpan_neon
#define _op_copy_rel_pt_pas_caa_dpan_neon _op_copy_pt_pas_caa_dpan_neon
#define _op_copy_rel_pt_pan_caa_dpan_neon _op_copy_pt_pan_caa_dpan_neon


static void
init_copy_rel_pixel_color_pt_funcs_neon(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_pt_p_c_dp_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_pt_pas_c_dp_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC][DP][CPU_NEON] = _op_copy_rel_pt_pan_c_dp_neon;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_pt_p_can_dp_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_pt_pas_can_dp_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_NEON] = _op_copy_rel_pt_pan_can_dp_neon;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_pt_p_caa_dp_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_pt_pas_caa_dp_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_NEON] = _op_copy_rel_pt_pan_caa_dp_neon;

   op_copy_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_pt_p_c_dpan_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_pt_pas_c_dpan_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_NEON] = _op_copy_rel_pt_pan_c_dpan_neon;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pt_p_can_dpan_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pt_pas_can_dpan_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_NEON] = _op_copy_rel_pt_pan_can_dpan_neon;
   op_copy_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pt_p_caa_dpan_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pt_pas_caa_dpan_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_NEON] = _op_copy_rel_pt_pan_caa_dpan_neon;
}
#endif
