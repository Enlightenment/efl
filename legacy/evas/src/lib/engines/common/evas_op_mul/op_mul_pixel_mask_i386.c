
/* mul pixel x mask --> dst */

#ifdef BUILD_MMX
static void
_op_mul_p_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   MOV_A2R(ALPHA_255, mm5)
   pxor_r2r(mm0, mm0);
   while (d < e) {
	c = *m;
	switch(c)
	  {
	    case 0:
		break;
	    case 255:
		MOV_P2R(*d, mm1, mm0)
		MOV_P2R(*s, mm2, mm0)
		MUL4_SYM_R2R(mm2, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	    default:
		c++;
		MOV_A2R(c, mm1)
		c = ~(*s);
		MOV_P2R(c, mm3, mm0)
		MUL4_256_R2R(mm3, mm1)
		movq_r2r(mm5, mm4);
		psubw_r2r(mm1, mm4);
		MOV_P2R(*d, mm1, mm0)
		MUL4_SYM_R2R(mm4, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	s++;  m++;  d++;
     }
}

#define _op_mul_pas_mas_dp_mmx _op_mul_p_mas_dp_mmx
#define _op_mul_pan_mas_dp_mmx _op_mul_p_mas_dp_mmx

#define _op_mul_p_mas_dpan_mmx _op_mul_p_mas_dp_mmx
#define _op_mul_pas_mas_dpan_mmx _op_mul_pas_mas_dp_mmx
#define _op_mul_pan_mas_dpan_mmx _op_mul_pan_mas_dp_mmx

static void
init_mul_pixel_mask_span_funcs_mmx(void)
{
   op_mul_span_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_mul_p_mas_dp_mmx;
   op_mul_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_mul_pas_mas_dp_mmx;
   op_mul_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_mul_pan_mas_dp_mmx;

   op_mul_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_mul_p_mas_dpan_mmx;
   op_mul_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_mul_pas_mas_dpan_mmx;
   op_mul_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_mul_pan_mas_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
init_mul_pixel_mask_pt_funcs_mmx(void)
{
}
#endif
