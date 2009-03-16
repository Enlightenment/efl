
/* copy pixel x mask --> dst */

#ifdef BUILD_NEON
static void
_op_copy_p_mas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   int color;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        color = *m;
                        switch(color)
                          {
                          case 0:
                             break;
                          case 255:
                             *d = *s;
                             break;
                          default:
                             color++;
                             *d = INTERP_256(color, *s, *d);
                             break;
                          }
                        m++;  s++;  d++;
                     });
}

#define _op_copy_pan_mas_dp_neon _op_copy_p_mas_dp_neon
#define _op_copy_pas_mas_dp_neon _op_copy_p_mas_dp_neon

#define _op_copy_p_mas_dpan_neon _op_copy_p_mas_dp_neon
#define _op_copy_pan_mas_dpan_neon _op_copy_p_mas_dpan_neon
#define _op_copy_pas_mas_dpan_neon _op_copy_p_mas_dpan_neon

static void
init_copy_pixel_mask_span_funcs_neon(void)
{
   op_copy_span_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_p_mas_dp_neon;
   op_copy_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_pan_mas_dp_neon;
   op_copy_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_pas_mas_dp_neon;

   op_copy_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_p_mas_dpan_neon;
   op_copy_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_pan_mas_dpan_neon;
   op_copy_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_pas_mas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void
_op_copy_pt_p_mas_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   *d = INTERP_256(m + 1, s, *d);
}

#define _op_copy_pt_pan_mas_dp_neon _op_copy_pt_p_mas_dp_neon
#define _op_copy_pt_pas_mas_dp_neon _op_copy_pt_p_mas_dp_neon

#define _op_copy_pt_p_mas_dpan_neon _op_copy_pt_p_mas_dp_neon
#define _op_copy_pt_pan_mas_dpan_neon _op_copy_pt_p_mas_dpan_neon
#define _op_copy_pt_pas_mas_dpan_neon _op_copy_pt_p_mas_dpan_neon

static void
init_copy_pixel_mask_pt_funcs_neon(void)
{
   op_copy_pt_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_pt_p_mas_dp_neon;
   op_copy_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_pt_pan_mas_dp_neon;
   op_copy_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_pt_pas_mas_dp_neon;

   op_copy_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_p_mas_dpan_neon;
   op_copy_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_pan_mas_dpan_neon;
   op_copy_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_pt_pas_mas_dpan_neon;
}
#endif

/*-----*/

/* copy_rel pixel x mask --> dst */

#ifdef BUILD_NEON
static void 
_op_copy_rel_p_mas_dp_neon(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   // FIXME: neon-it
   DATA32 *e;
   int color;
   UNROLL8_PLD_WHILE(d, l, e,
                     {
                        color = *m;
                        switch(color)
                          {
                          case 0:
                             break;
                          case 255:
                             *d = MUL_SYM(*d >> 24, *s);
                             break;
                          default:
                             c = MUL_SYM(*d >> 24, *s);
                             l++;
                             *d = INTERP_256(l, c, *d);
                             break;
                          }
                        m++;  s++;  d++;
                     });
}

#define _op_copy_rel_pan_mas_dp_neon _op_copy_rel_p_mas_dp_neon
#define _op_copy_rel_pas_mas_dp_neon _op_copy_rel_p_mas_dp_neon

#define _op_copy_rel_p_mas_dpan_neon _op_copy_p_mas_dpan_neon
#define _op_copy_rel_pan_mas_dpan_neon _op_copy_pan_mas_dpan_neon
#define _op_copy_rel_pas_mas_dpan_neon _op_copy_pas_mas_dpan_neon

static void
init_copy_rel_pixel_mask_span_funcs_neon(void)
{
   op_copy_rel_span_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_p_mas_dp_neon;
   op_copy_rel_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_pan_mas_dp_neon;
   op_copy_rel_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_pas_mas_dp_neon;

   op_copy_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_p_mas_dpan_neon;
   op_copy_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pan_mas_dpan_neon;
   op_copy_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pas_mas_dpan_neon;
}
#endif

#ifdef BUILD_NEON
static void 
_op_copy_rel_pt_p_mas_dp_neon(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   c = MUL_SYM(*d >> 24, s);
   *d = INTERP_256(m + 1, c, *d);
}


#define _op_copy_rel_pt_pan_mas_dp_neon _op_copy_rel_pt_p_mas_dp_neon
#define _op_copy_rel_pt_pas_mas_dp_neon _op_copy_rel_pt_p_mas_dp_neon

#define _op_copy_rel_pt_p_mas_dpan_neon _op_copy_pt_p_mas_dpan_neon
#define _op_copy_rel_pt_pan_mas_dpan_neon _op_copy_pt_pan_mas_dpan_neon
#define _op_copy_rel_pt_pas_mas_dpan_neon _op_copy_pt_pas_mas_dpan_neon

static void
init_copy_rel_pixel_mask_pt_funcs_neon(void)
{
   op_copy_rel_pt_funcs[SP][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_p_mas_dp_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_pan_mas_dp_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_NEON] = _op_copy_rel_pt_pas_mas_dp_neon;

   op_copy_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_p_mas_dpan_neon;
   op_copy_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_pan_mas_dpan_neon;
   op_copy_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_NEON] = _op_copy_rel_pt_pas_mas_dpan_neon;
}
#endif

