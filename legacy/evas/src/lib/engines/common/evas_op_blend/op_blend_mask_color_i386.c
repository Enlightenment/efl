
/* blend mask x color -> dst */

#ifdef BUILD_MMX
static void
_op_blend_mas_c_dp_mmx(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   MOV_P2R(c, mm2, mm0)
   c = 256 - (c >> 24);
   MOV_A2R(c, mm4)
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    case 255:
		MOV_P2R(*d, mm1, mm0)
		MUL4_256_R2R(mm4, mm1)
		paddw_r2r(mm2, mm1);
		MOV_R2P(mm1, *d, mm0)
		break;
	    default:
		l++;
		MOV_A2R(l, mm3)
		MUL4_256_R2R(mm2, mm3)

		MOV_RA2R(mm3, mm1)
		movq_r2r(mm6, mm7);
		psubw_r2r(mm1, mm7);

		MOV_P2R(*d, mm1, mm0)
		MUL4_256_R2R(mm7, mm1)

		paddw_r2r(mm3, mm1);
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

static void
_op_blend_mas_can_dp_mmx(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(c, mm2, mm0)
   MOV_A2R(ALPHA_255, mm5)
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    case 255:
		*d = c;
		break;
	    default:
		l++;
		MOV_A2R(l, mm3)
		MOV_P2R(*d, mm1, mm0)
		movq_r2r(mm2, mm4);
		INTERP_256_R2R(mm3, mm4, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

#define _op_blend_mas_cn_dp_mmx _op_blend_mas_can_dp_mmx
#define _op_blend_mas_caa_dp_mmx _op_blend_mas_c_dp_mmx

#define _op_blend_mas_c_dpan_mmx _op_blend_mas_c_dp_mmx
#define _op_blend_mas_cn_dpan_mmx _op_blend_mas_cn_dp_mmx
#define _op_blend_mas_can_dpan_mmx _op_blend_mas_can_dp_mmx
#define _op_blend_mas_caa_dpan_mmx _op_blend_mas_caa_dp_mmx

static void
init_blend_mask_color_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_mas_c_dp_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_mas_cn_dp_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_mas_can_dp_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_mas_caa_dp_mmx;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_mas_c_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_mas_cn_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_mas_can_dpan_mmx;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_mas_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_mas_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	s = m + 1;
	MOV_A2R(s, mm3)
	MOV_A2R(ALPHA_256, mm6)
	pxor_r2r(mm0, mm0);
	MOV_P2R(c, mm2, mm0)
	MUL4_256_R2R(mm2, mm3)

	MOV_RA2R(mm3, mm1)
	psubw_r2r(mm1, mm6);

	MOV_P2R(*d, mm1, mm0)
	MUL4_256_R2R(mm6, mm1)

	paddw_r2r(mm3, mm1);
	MOV_R2P(mm1, *d, mm0)
}


#define _op_blend_pt_mas_cn_dp_mmx _op_blend_pt_mas_c_dp_mmx
#define _op_blend_pt_mas_can_dp_mmx _op_blend_pt_mas_c_dp_mmx
#define _op_blend_pt_mas_caa_dp_mmx _op_blend_pt_mas_c_dp_mmx

#define _op_blend_pt_mas_c_dpan_mmx _op_blend_pt_mas_c_dp_mmx
#define _op_blend_pt_mas_cn_dpan_mmx _op_blend_pt_mas_cn_dp_mmx
#define _op_blend_pt_mas_can_dpan_mmx _op_blend_pt_mas_can_dp_mmx
#define _op_blend_pt_mas_caa_dpan_mmx _op_blend_pt_mas_caa_dp_mmx

static void
init_blend_mask_color_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_pt_mas_c_dp_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_mas_cn_dp_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_pt_mas_can_dp_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_pt_mas_caa_dp_mmx;

   op_blend_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_pt_mas_c_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_mas_cn_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_pt_mas_can_dpan_mmx;
   op_blend_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_pt_mas_caa_dpan_mmx;
}
#endif

/*-----*/

/* blend_rel mask x color -> dst */

#ifdef BUILD_MMX
static void
_op_blend_rel_mas_c_dp_mmx(DATA32 *s __UNUSED__, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   MOV_A2R(ALPHA_255, mm5)
   MOV_P2R(c, mm2, mm0)
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    default:
		l++;
		MOV_A2R(l, mm3)
		MUL4_256_R2R(mm2, mm3)

		MOV_RA2R(mm3, mm1)
		movq_r2r(mm6, mm7);
		psubw_r2r(mm1, mm7);

		MOV_P2R(*d, mm1, mm0)
		MOV_RA2R(mm1, mm4)
		MUL4_256_R2R(mm7, mm1)

		MUL4_SYM_R2R(mm4, mm3, mm5)

		paddw_r2r(mm3, mm1);
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

#define _op_blend_rel_mas_cn_dp_mmx _op_blend_rel_mas_c_dp_mmx
#define _op_blend_rel_mas_can_dp_mmx _op_blend_rel_mas_c_dp_mmx
#define _op_blend_rel_mas_caa_dp_mmx _op_blend_rel_mas_c_dp_mmx

#define _op_blend_rel_mas_c_dpan_mmx _op_blend_mas_c_dpan_mmx
#define _op_blend_rel_mas_cn_dpan_mmx _op_blend_mas_cn_dpan_mmx
#define _op_blend_rel_mas_can_dpan_mmx _op_blend_mas_can_dpan_mmx
#define _op_blend_rel_mas_caa_dpan_mmx _op_blend_mas_caa_dpan_mmx

static void
init_blend_rel_mask_color_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_rel_mas_c_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_mas_cn_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_rel_mas_can_dp_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_rel_mas_caa_dp_mmx;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_rel_mas_c_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_mas_cn_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_mas_can_dpan_mmx;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_mas_caa_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_rel_pt_mas_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_256, mm6)
	MOV_A2R(ALPHA_255, mm5)
	s = m + 1;
	MOV_A2R(s, mm3)
	MOV_P2R(c, mm2, mm0)
	MUL4_256_R2R(mm2, mm3)

	MOV_RA2R(mm3, mm1)
	psubw_r2r(mm1, mm6);

	MOV_P2R(*d, mm1, mm0)
	MOV_RA2R(mm1, mm4)
	MUL4_256_R2R(mm6, mm1)

	MUL4_SYM_R2R(mm4, mm3, mm5)

	paddw_r2r(mm3, mm1);
	MOV_R2P(mm1, *d, mm0)
}

#define _op_blend_rel_pt_mas_cn_dp_mmx _op_blend_rel_pt_mas_c_dp_mmx
#define _op_blend_rel_pt_mas_can_dp_mmx _op_blend_rel_pt_mas_c_dp_mmx
#define _op_blend_rel_pt_mas_caa_dp_mmx _op_blend_rel_pt_mas_c_dp_mmx

#define _op_blend_rel_pt_mas_c_dpan_mmx _op_blend_pt_mas_c_dpan_mmx
#define _op_blend_rel_pt_mas_cn_dpan_mmx _op_blend_pt_mas_cn_dpan_mmx
#define _op_blend_rel_pt_mas_can_dpan_mmx _op_blend_pt_mas_can_dpan_mmx
#define _op_blend_rel_pt_mas_caa_dpan_mmx _op_blend_pt_mas_caa_dpan_mmx

static void
init_blend_rel_mask_color_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP][CPU_MMX] = _op_blend_rel_pt_mas_c_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_mas_cn_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP][CPU_MMX] = _op_blend_rel_pt_mas_can_dp_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP][CPU_MMX] = _op_blend_rel_pt_mas_caa_dp_mmx;

   op_blend_rel_pt_funcs[SP_N][SM_AS][SC][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_c_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_cn_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_can_dpan_mmx;
   op_blend_rel_pt_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pt_mas_caa_dpan_mmx;
}
#endif
