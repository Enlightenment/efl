
/* blend pixel x mask --> dst */

#ifdef BUILD_MMX
static void
_op_blend_pas_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   while (d < e) {
	l = (*s >> 24);
	switch(*m & l)
	  {
	    case 0:
		break;
	    case 255:
		*d = *s;
		break;
	    default:
		l = 1 + *m;
		MOV_A2R(l, mm3)
		MOV_P2R(*s, mm2, mm0)
		MUL4_256_R2R(mm3, mm2)

		MOV_RA2R(mm2, mm1)
		movq_r2r(mm6, mm3);
		psubw_r2r(mm1, mm3);

		MOV_P2R(*d, mm1, mm0)
		MUL4_256_R2R(mm3, mm1)

		paddw_r2r(mm2, mm1);
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

static void
_op_blend_pan_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e = d + l;
   MOV_A2R(ALPHA_255, mm5)
   pxor_r2r(mm0, mm0);
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    case 255:
		*d = *s;
		break;
	    default:
		l++;
		MOV_A2R(l, mm3)
		MOV_P2R(*s, mm2, mm0)
		MOV_P2R(*d, mm1, mm0)
		INTERP_256_R2R(mm3, mm2, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_blend_p_mas_dp_mmx _op_blend_pas_mas_dp_mmx

#define _op_blend_p_mas_dpan_mmx _op_blend_p_mas_dp_mmx
#define _op_blend_pan_mas_dpan_mmx _op_blend_pan_mas_dp_mmx
#define _op_blend_pas_mas_dpan_mmx _op_blend_pas_mas_dp_mmx

static void
init_blend_pixel_mask_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_p_mas_dp_mmx;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pas_mas_dp_mmx;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pan_mas_dp_mmx;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_p_mas_dpan_mmx;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pas_mas_dpan_mmx;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pan_mas_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_p_mas_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = m + 1;
	MOV_A2R(c, mm3)
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_256, mm6)
	MOV_P2R(s, mm2, mm0)
	MUL4_256_R2R(mm3, mm2)

	MOV_RA2R(mm2, mm1)
	psubw_r2r(mm1, mm6);

	MOV_P2R(*d, mm1, mm0)
	MUL4_256_R2R(mm6, mm1)

	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
}

#define _op_blend_pt_pan_mas_dp_mmx _op_blend_pt_p_mas_dp_mmx
#define _op_blend_pt_pas_mas_dp_mmx _op_blend_pt_p_mas_dp_mmx

#define _op_blend_pt_p_mas_dpan_mmx _op_blend_pt_p_mas_dp_mmx
#define _op_blend_pt_pas_mas_dpan_mmx _op_blend_pt_pas_mas_dp_mmx
#define _op_blend_pt_pan_mas_dpan_mmx _op_blend_pt_pan_mas_dp_mmx

static void
init_blend_pixel_mask_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_p_mas_dp_mmx;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_pas_mas_dp_mmx;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_blend_pt_pan_mas_dp_mmx;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_p_mas_dpan_mmx;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_pas_mas_dpan_mmx;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_pan_mas_dpan_mmx;
}
#endif

/*-----*/

/* blend_rel pixel x mask -> dst */

#ifdef BUILD_MMX

#define _op_blend_rel_p_mas_dpan_mmx _op_blend_p_mas_dpan_mmx
#define _op_blend_rel_pas_mas_dpan_mmx _op_blend_pas_mas_dpan_mmx
#define _op_blend_rel_pan_mas_dpan_mmx _op_blend_pan_mas_dpan_mmx

static void
init_blend_rel_pixel_mask_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_p_mas_dpan_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pas_mas_dpan_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pan_mas_dpan_mmx;
}
#endif

#ifdef BUILD_MMX

#define _op_blend_rel_pt_p_mas_dpan_mmx _op_blend_pt_p_mas_dpan_mmx
#define _op_blend_rel_pt_pas_mas_dpan_mmx _op_blend_pt_pas_mas_dpan_mmx
#define _op_blend_rel_pt_pan_mas_dpan_mmx _op_blend_pt_pan_mas_dpan_mmx

static void
init_blend_rel_pixel_mask_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_p_mas_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_pas_mas_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_pan_mas_dpan_mmx;
}
#endif
