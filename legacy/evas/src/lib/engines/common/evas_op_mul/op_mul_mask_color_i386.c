
/* mul mask x color -> dst */

#ifdef BUILD_MMX
static void
_op_mul_mas_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   MOV_P2R(c, mm2, mm0)
   c = ~c;
   MOV_P2R(c, mm3, mm0)
   MOV_A2R(ALPHA_255, mm5)
   pxor_r2r(mm0, mm0);
   while (d < e) {
	DATA32 a = *m;
	switch(a)
	  {
	    case 0:
		break;
	    case 255:
		MOV_P2R(*d, mm1, mm0)
		MUL4_SYM_R2R(mm2, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	    default:
		a++;
		MOV_A2R(a, mm1)
		MUL4_256_R2R(mm3, mm1)
		movq_r2r(mm5, mm4);
		psubw_r2r(mm1, mm4);
		MOV_P2R(*d, mm1, mm0)
		MUL4_SYM_R2R(mm4, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

#define _op_mul_mas_can_dp_mmx _op_mul_mas_c_dp_mmx
#define _op_mul_mas_caa_dp_mmx _op_mul_mas_c_dp_mmx

#define _op_mul_mas_c_dpan_mmx _op_mul_mas_c_dp_mmx
#define _op_mul_mas_can_dpan_mmx _op_mul_mas_can_dp_mmx
#define _op_mul_mas_caa_dpan_mmx _op_mul_mas_caa_dp_mmx

static void
init_mul_mask_color_span_funcs_mmx(void)
{
   op_mul_span_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_mul_mas_c_dp_mmx;
   op_mul_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_mul_mas_can_dp_mmx;
   op_mul_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_mul_mas_caa_dp_mmx;

   op_mul_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_mul_mas_c_dpan_mmx;
   op_mul_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_mul_mas_can_dpan_mmx;
   op_mul_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_mul_mas_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_mul_pt_mas_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = m + 1;
	c = ~c;
	MOV_P2R(c, mm3, mm0)
	MOV_A2R(ALPHA_255, mm4)
	pxor_r2r(mm0, mm0);
	MOV_A2R(s, mm1)
	MUL4_256_R2R(mm3, mm1)
	psubw_r2r(mm1, mm4);
	MOV_P2R(*d, mm1, mm0)
	MUL4_SYM_R2R(mm4, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
}

#define _op_mul_pt_mas_can_dp_mmx _op_mul_pt_mas_c_dp_mmx
#define _op_mul_pt_mas_caa_dp_mmx _op_mul_pt_mas_c_dp_mmx

#define _op_mul_pt_mas_c_dpan_mmx _op_mul_pt_mas_c_dp_mmx
#define _op_mul_pt_mas_can_dpan_mmx _op_mul_pt_mas_can_dp_mmx
#define _op_mul_pt_mas_caa_dpan_mmx _op_mul_pt_mas_caa_dp_mmx

static void
init_mul_mask_color_pt_funcs_mmx(void)
{
   op_mul_pt_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_mul_pt_mas_c_dp_mmx;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_mul_pt_mas_can_dp_mmx;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_mul_pt_mas_caa_dp_mmx;

   op_mul_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_mul_pt_mas_c_dpan_mmx;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_mul_pt_mas_can_dpan_mmx;
   op_mul_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_mul_pt_mas_caa_dpan_mmx;
}
#endif
