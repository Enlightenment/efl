
/* blend pixel --> dst */

#ifdef BUILD_C
static void
_op_blend_p_dp(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        alpha = 256 - (*s >> 24);
                        *d = *s++ + MUL_256(alpha, *d);
                        d++;
                     });
}

static void
_op_blend_pas_dp(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        switch (*s & 0xff000000)
                          {
                          case 0:
                             break;
                          case 0xff000000:
                             *d = *s;
                             break;
                          default:
                             alpha = 256 - (*s >> 24);
                             *d = *s + MUL_256(alpha, *d);
                             break;
                          }
                        s++;  d++;
                     });
}

#define _op_blend_pan_dp NULL

#define _op_blend_p_dpan _op_blend_p_dp
#define _op_blend_pas_dpan _op_blend_pas_dp
#define _op_blend_pan_dpan _op_blend_pan_dp

static void
init_blend_pixel_span_funcs_c(void)
{
   op_blend_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_p_dp;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_pas_dp;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_pan_dp;

   op_blend_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_p_dpan;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pas_dpan;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pan_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_blend_pt_p_dp(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   c = 256 - (s >> 24);
   *d = s + MUL_256(c, *d);
}

#define _op_blend_pt_pas_dp _op_blend_pt_p_dp
#define _op_blend_pt_pan_dp NULL

#define _op_blend_pt_p_dpan _op_blend_pt_p_dp
#define _op_blend_pt_pan_dpan _op_blend_pt_pan_dp
#define _op_blend_pt_pas_dpan _op_blend_pt_pas_dp

static void
init_blend_pixel_pt_funcs_c(void)
{
   op_blend_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_pt_p_dp;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_pt_pas_dp;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_pt_pan_dp;

   op_blend_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pt_p_dpan;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pt_pas_dpan;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_pt_pan_dpan;
}
#endif

/*-----*/

/* blend_rel pixel -> dst */

#ifdef BUILD_C
static void
_op_blend_rel_p_dp(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        alpha = 256 - (*s >> 24);
                        c = 1 + (*d >> 24);
                        *d = MUL_256(c, *s) + MUL_256(alpha, *d);
                        d++;
                        s++;
                     });
}

static void
_op_blend_rel_pan_dp(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {
   DATA32 *e;
   int alpha;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        c = 1 + (*d >> 24);
                        *d++ = MUL_256(c, *s);
                        s++;
                     });
}

#define _op_blend_rel_pas_dp _op_blend_rel_p_dp

#define _op_blend_rel_p_dpan _op_blend_p_dpan
#define _op_blend_rel_pan_dpan _op_blend_pan_dpan
#define _op_blend_rel_pas_dpan _op_blend_pas_dpan

static void
init_blend_rel_pixel_span_funcs_c(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_p_dp;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pas_dp;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pan_dp;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_p_dpan;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pas_dpan;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pan_dpan;
}
#endif

#ifdef BUILD_C
static void
_op_blend_rel_pt_p_dp(DATA32 s, DATA8 m __UNUSED__, DATA32 c, DATA32 *d) {
   c = 256 - (s >> 24);
   *d = MUL_SYM(*d >> 24, s) + MUL_256(c, *d);
}

static void
_op_blend_rel_pt_pan_dp(DATA32 s, DATA8 m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d) {
   *d = MUL_SYM(*d >> 24, s);
}

#define _op_blend_rel_pt_pas_dp _op_blend_rel_pt_p_dp

#define _op_blend_rel_pt_p_dpan _op_blend_pt_p_dpan
#define _op_blend_rel_pt_pan_dpan _op_blend_pt_pan_dpan
#define _op_blend_rel_pt_pas_dpan _op_blend_pt_pas_dpan

static void
init_blend_rel_pixel_pt_funcs_c(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pt_p_dp;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pt_pas_dp;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_C] = _op_blend_rel_pt_pan_dp;

   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_p_dpan;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_pas_dpan;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_C] = _op_blend_rel_pt_pan_dpan;
}
#endif
