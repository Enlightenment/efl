
/* copy pixel --> dst */

#ifdef BUILD_C
static void
_op_copy_p_dp(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   memcpy(d, s, l * sizeof(DATA32));
}

#define _op_copy_pan_dp _op_copy_p_dp
#define _op_copy_pas_dp _op_copy_p_dp

#define _op_copy_p_dpan _op_copy_p_dp
#define _op_copy_pan_dpan _op_copy_pan_dp
#define _op_copy_pas_dpan _op_copy_pas_dp

static void
init_copy_pixel_span_funcs_c(void)
{
   op_copy_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_p_dp;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_pan_dp;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_pas_dp;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_p_dpan;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pan_dpan;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pas_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_copy_pt_p_dp(DATA32 s, DATA8 m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d) {
   *d = s;
}

#define _op_copy_pt_pan_dp _op_copy_pt_p_dp
#define _op_copy_pt_pas_dp _op_copy_pt_p_dp

#define _op_copy_pt_p_dpan _op_copy_pt_p_dp
#define _op_copy_pt_pan_dpan _op_copy_pt_pan_dp
#define _op_copy_pt_pas_dpan _op_copy_pt_pas_dp

static void
init_copy_pixel_pt_funcs_c(void)
{
   op_copy_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_pt_p_dp;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_pt_pan_dp;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_pt_pas_dp;

   op_copy_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pt_p_dpan;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pt_pan_dpan;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_pt_pas_dpan;
}
#endif

/*-----*/

/* copy_rel pixel --> dst */

#ifdef BUILD_C
static void
_op_copy_rel_p_dp(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        *d = MUL_SYM(*d >> 24, *s);
                        d++; s++;
                     });
}


#define _op_copy_rel_pas_dp _op_copy_rel_p_dp
#define _op_copy_rel_pan_dp _op_copy_rel_p_dp

#define _op_copy_rel_p_dpan _op_copy_p_dpan
#define _op_copy_rel_pan_dpan _op_copy_pan_dpan
#define _op_copy_rel_pas_dpan _op_copy_pas_dpan

static void
init_copy_rel_pixel_span_funcs_c(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_p_dp;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pan_dp;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pas_dp;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_p_dpan;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pan_dpan;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pas_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_copy_rel_pt_p_dp(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   c = 1 + (*d >> 24);
   *d = MUL_256(c, s);
}


#define _op_copy_rel_pt_pan_dp _op_copy_rel_pt_p_dp
#define _op_copy_rel_pt_pas_dp _op_copy_rel_pt_p_dp

#define _op_copy_rel_pt_p_dpan _op_copy_pt_p_dpan
#define _op_copy_rel_pt_pan_dpan _op_copy_pt_pan_dpan
#define _op_copy_rel_pt_pas_dpan _op_copy_pt_pas_dpan

static void
init_copy_rel_pixel_pt_funcs_c(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pt_p_dp;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pt_pan_dp;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_copy_rel_pt_pas_dp;

   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_p_dpan;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_pan_dpan;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_copy_rel_pt_pas_dpan;
}
#endif
