
/* copy pixel --> dst */

#ifdef BUILD_NEON
static void
_op_copy_p_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   memcpy(d, s, l * sizeof(DATA32));
}

#define _op_copy_pan_dp_neon _op_copy_p_dp_neon
#define _op_copy_pas_dp_neon _op_copy_p_dp_neon

#define _op_copy_p_dpan_neon _op_copy_p_dp_neon
#define _op_copy_pan_dpan_neon _op_copy_pan_dp_neon
#define _op_copy_pas_dpan_neon _op_copy_pas_dp_neon

static void
init_copy_pixel_span_funcs_neon(void)
{
   op_copy_span_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_p_dp_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pan_dp_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pas_dp_neon;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_p_dpan_neon;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pan_dpan_neon;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_p_dp_neon(DATA32 s, DATA8 m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d) {
   *d = s;
}

#define _op_copy_pt_pan_dp_neon _op_copy_pt_p_dp_neon
#define _op_copy_pt_pas_dp_neon _op_copy_pt_p_dp_neon

#define _op_copy_pt_p_dpan_neon _op_copy_pt_p_dp_neon
#define _op_copy_pt_pan_dpan_neon _op_copy_pt_pan_dp_neon
#define _op_copy_pt_pas_dpan_neon _op_copy_pt_pas_dp_neon

static void
init_copy_pixel_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_p_dp_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_pan_dp_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_pt_pas_dp_neon;

   op_copy_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_p_dpan_neon;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_pan_dpan_neon;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_pas_dpan_neon;
}
#endif

/*-----*/

/* copy_rel pixel --> dst */

#ifdef BUILD_NEON
static void
_op_copy_rel_p_dp_neon(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = MUL_SYM(*d >> 24, c);
                        d++;
                     });
}


#define _op_copy_rel_pas_dp_neon _op_copy_rel_p_dp_neon
#define _op_copy_rel_pan_dp_neon _op_copy_rel_p_dp_neon

#define _op_copy_rel_p_dpan_neon _op_copy_p_dpan_neon
#define _op_copy_rel_pan_dpan_neon _op_copy_pan_dpan_neon
#define _op_copy_rel_pas_dpan_neon _op_copy_pas_dpan_neon

static void
init_copy_rel_pixel_span_funcs_neon(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_p_dp_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pan_dp_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pas_dp_neon;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_p_dpan_neon;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pan_dpan_neon;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_rel_pt_p_dp_neon(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   s = 1 + (*d >> 24);
   *d = MUL_256(s, c);
}


#define _op_copy_rel_pt_pan_dp_neon _op_copy_rel_pt_p_dp_neon
#define _op_copy_rel_pt_pas_dp_neon _op_copy_rel_pt_p_dp_neon

#define _op_copy_rel_pt_p_dpan_neon _op_copy_pt_p_dpan_neon
#define _op_copy_rel_pt_pan_dpan_neon _op_copy_pt_pan_dpan_neon
#define _op_copy_rel_pt_pas_dpan_neon _op_copy_pt_pas_dpan_neon

static void
init_copy_rel_pixel_pt_funcs_neon(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_p_dp_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_pan_dp_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_pas_dp_neon;

   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_p_dpan_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_pan_dpan_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_pas_dpan_neon;
}
#endif
