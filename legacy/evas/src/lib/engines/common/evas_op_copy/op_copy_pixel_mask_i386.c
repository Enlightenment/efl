
/* copy pixel x mask --> dst */

#ifdef BUILD_MMX
static void
_op_copy_p_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
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
		INTERP_256_R2R(mm3, mm2, mm1, mm5);
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  d++;
     }
}

#define _op_copy_pan_mas_dp_mmx _op_copy_p_mas_dp_mmx
#define _op_copy_pas_mas_dp_mmx _op_copy_p_mas_dp_mmx

#define _op_copy_p_mas_dpan_mmx _op_copy_p_mas_dp_mmx
#define _op_copy_pan_mas_dpan_mmx _op_copy_p_mas_dpan_mmx
#define _op_copy_pas_mas_dpan_mmx _op_copy_p_mas_dpan_mmx

static void
init_copy_pixel_mask_span_funcs_mmx(void)
{
   op_copy_span_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_p_mas_dp_mmx;
   op_copy_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_pan_mas_dp_mmx;
   op_copy_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_pas_mas_dp_mmx;

   op_copy_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_p_mas_dpan_mmx;
   op_copy_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_pan_mas_dpan_mmx;
   op_copy_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_pas_mas_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_copy_pt_p_mas_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = m + 1;
	MOV_A2R(ALPHA_255, mm5)
	MOV_A2R(c, mm3)
	pxor_r2r(mm0, mm0);
	MOV_P2R(s, mm2, mm0)
	MOV_P2R(*d, mm1, mm0)
	INTERP_256_R2R(mm3, mm2, mm1, mm5);
	MOV_R2P(mm1, *d, mm0)
}

#define _op_copy_pt_pan_mas_dp_mmx _op_copy_pt_p_mas_dp_mmx
#define _op_copy_pt_pas_mas_dp_mmx _op_copy_pt_p_mas_dp_mmx

#define _op_copy_pt_p_mas_dpan_mmx _op_copy_pt_p_mas_dp_mmx
#define _op_copy_pt_pan_mas_dpan_mmx _op_copy_pt_p_mas_dpan_mmx
#define _op_copy_pt_pas_mas_dpan_mmx _op_copy_pt_p_mas_dpan_mmx

static void
init_copy_pixel_mask_pt_funcs_mmx(void)
{
   op_copy_pt_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_pt_p_mas_dp_mmx;
   op_copy_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_pt_pan_mas_dp_mmx;
   op_copy_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_pt_pas_mas_dp_mmx;

   op_copy_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_pt_p_mas_dpan_mmx;
   op_copy_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_pt_pan_mas_dpan_mmx;
   op_copy_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_pt_pas_mas_dpan_mmx;
}
#endif

/*-----*/

/* copy_rel pixel x mask --> dst */

#ifdef BUILD_MMX
static void 
_op_copy_rel_p_mas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   while (d < e) {
	l = *m;
	switch(l)
	  {
	    case 0:
		break;
	    case 255:
		MOV_P2R(*s, mm2, mm0)
		MOV_PA2R(*d, mm1)
		MUL4_SYM_R2R(mm2, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	    default:
		l++;
		MOV_P2R(*s, mm3, mm0)
		MOV_P2R(*d, mm1, mm0)
		MOV_RA2R(mm1, mm2)
		MUL4_SYM_R2R(mm3, mm2, mm5)
		MOV_A2R(l, mm3)
		INTERP_256_R2R(mm3, mm2, mm1, mm5)
		MOV_R2P(mm1, *d, mm0)
		break;
	  }
	m++;  s++;  d++;
     }
}

#define _op_copy_rel_pan_mas_dp_mmx _op_copy_rel_p_mas_dp_mmx
#define _op_copy_rel_pas_mas_dp_mmx _op_copy_rel_p_mas_dp_mmx

#define _op_copy_rel_p_mas_dpan_mmx _op_copy_p_mas_dpan_mmx
#define _op_copy_rel_pan_mas_dpan_mmx _op_copy_pan_mas_dpan_mmx
#define _op_copy_rel_pas_mas_dpan_mmx _op_copy_pas_mas_dpan_mmx

static void
init_copy_rel_pixel_mask_span_funcs_mmx(void)
{
   op_copy_rel_span_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_rel_p_mas_dp_mmx;
   op_copy_rel_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_rel_pan_mas_dp_mmx;
   op_copy_rel_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_rel_pas_mas_dp_mmx;

   op_copy_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_p_mas_dpan_mmx;
   op_copy_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pan_mas_dpan_mmx;
   op_copy_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pas_mas_dpan_mmx;
}
#endif

#ifdef BUILD_MMX
static void 
_op_copy_rel_pt_p_mas_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	c = m + 1;
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_255, mm5)
	MOV_P2R(s, mm3, mm0)
	MOV_P2R(*d, mm1, mm0)
	MOV_RA2R(mm1, mm2)
	MUL4_SYM_R2R(mm3, mm2, mm5)
	MOV_A2R(c, mm3)
	INTERP_256_R2R(mm3, mm2, mm1, mm5)
	MOV_R2P(mm1, *d, mm0)
}


#define _op_copy_rel_pt_pan_mas_dp_mmx _op_copy_rel_pt_p_mas_dp_mmx
#define _op_copy_rel_pt_pas_mas_dp_mmx _op_copy_rel_pt_p_mas_dp_mmx

#define _op_copy_rel_pt_p_mas_dpan_mmx _op_copy_pt_p_mas_dpan_mmx
#define _op_copy_rel_pt_pan_mas_dpan_mmx _op_copy_pt_pan_mas_dpan_mmx
#define _op_copy_rel_pt_pas_mas_dpan_mmx _op_copy_pt_pas_mas_dpan_mmx

static void
init_copy_rel_pixel_mask_pt_funcs_mmx(void)
{
   op_copy_rel_pt_funcs[SP][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_rel_pt_p_mas_dp_mmx;
   op_copy_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_rel_pt_pan_mas_dp_mmx;
   op_copy_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_MMX] = _op_copy_rel_pt_pas_mas_dp_mmx;

   op_copy_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pt_p_mas_dpan_mmx;
   op_copy_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pt_pan_mas_dpan_mmx;
   op_copy_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pt_pas_mas_dpan_mmx;
}
#endif

