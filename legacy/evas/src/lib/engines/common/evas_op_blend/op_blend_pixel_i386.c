
/* blend pixel --> dst */

#ifdef BUILD_MMX
static void
_op_blend_p_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   while (d < e)
     {
	DATA32  a = *s & 0xff000000;
	MOV_MA2R(a, mm2)
	psllq_i2r(48, mm2);

	a = _evas_pow_lut[(a >> 16) + (*d >> 24)] << 24;
	MOV_MA2R(a, mm3)
	psrlq_i2r(16, mm3);

	por_r2r(mm2, mm3);
	psrlw_i2r(1, mm3);

	a = *s | 0xff000000;
	MOV_P2R(a, mm1, mm0)
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_pas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   while (d < e)
     {
	DATA32  a = *s & 0xff000000;
	switch(a)
	  {
	    case 0:
		break;
	    case 0xff000000:
		*d = *s;
		break;
	    default:
		MOV_MA2R(a, mm2)
		psllq_i2r(48, mm2);

		a = _evas_pow_lut[(a >> 16) + (*d >> 24)] << 24;
		MOV_MA2R(a, mm3)
		psrlq_i2r(16, mm3);

		por_r2r(mm2, mm3);
		psrlw_i2r(1, mm3);

		a = *s | 0xff000000;
		MOV_P2R(a, mm1, mm0)
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_p_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   while (d < e)
     {
	movd_m2r(*s, mm1);         // mm1 = [  ][  ][AR][GB] (SRC)

	MOV_RA2R(mm1, mm3)
	psrlw_i2r(1, mm3);         // mm3 = [AA/2][AA/2][AA/2][AA/2]
	psrlq_i2r(16, mm3);        // mm3 = [00][AA/2][AA/2][AA/2]

	punpcklbw_r2r(mm0, mm1);   // mm1 = [0A][0R][0G][0B]
	MOV_P2R(*d, mm2, mm0)      // mm2 = [0a][0r][0g][0b] (DST)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_pas_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		break;
	    case 0xff000000:
		*d = *s;
		break;
	    default:
		movd_m2r(*s, mm1);

		MOV_RA2R(mm1, mm3)
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);
		MOV_P2R(*d, mm2, mm0)

		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	s++;  d++;
     }
}

#define _op_blend_pan_dp_mmx NULL

#define _op_blend_pan_dpan_mmx _op_blend_pan_dp_mmx

#define _op_blend_p_dpas_mmx _op_blend_p_dp_mmx
#define _op_blend_pas_dpas_mmx _op_blend_pas_dp_mmx
#define _op_blend_pan_dpas_mmx _op_blend_pan_dp_mmx

static void
init_blend_pixel_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_blend_p_dp_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_blend_pas_dp_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_blend_pan_dp_mmx;

   op_blend_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_p_dpan_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_pas_dpan_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_pan_dpan_mmx;

   op_blend_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_p_dpas_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_pas_dpas_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_pan_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_p_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)
	c = s & 0xff000000;
	MOV_MA2R(c, mm2)
	psllq_i2r(48, mm2);

	c = _evas_pow_lut[(c >> 16) + (*d >> 24)] << 24;
	MOV_MA2R(c, mm3)
	psrlq_i2r(16, mm3);

	por_r2r(mm2, mm3);
	psrlw_i2r(1, mm3);

	s |= 0xff000000;
	MOV_P2R(s, mm1, mm0)
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

static void
_op_blend_pt_p_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	pxor_r2r(mm0, mm0);
	MOV_A2R(ALPHA_ONE, mm6)

	movd_m2r(s, mm1);
	MOV_RA2R(mm1, mm3)
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	punpcklbw_r2r(mm0, mm1);
	MOV_P2R(*d, mm2, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
}

#define _op_blend_pt_pan_dp_mmx NULL
#define _op_blend_pt_pas_dp_mmx _op_blend_pt_p_dp_mmx

#define _op_blend_pt_pan_dpan_mmx _op_blend_pt_pan_dp_mmx
#define _op_blend_pt_pas_dpan_mmx _op_blend_pt_pas_dp_mmx

#define _op_blend_pt_p_dpas_mmx _op_blend_pt_p_dp_mmx
#define _op_blend_pt_pas_dpas_mmx _op_blend_pt_pas_dp_mmx
#define _op_blend_pt_pan_dpas_mmx _op_blend_pt_pan_dp_mmx

static void
init_blend_pixel_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_blend_pt_p_dp_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_blend_pt_pas_dp_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_blend_pt_pan_dp_mmx;

   op_blend_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_p_dpan_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_pas_dpan_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_pt_pan_dpan_mmx;

   op_blend_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_pt_p_dpas_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_pt_pas_dpas_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_pt_pan_dpas_mmx;
}
#endif

/*-----*/

/* blend_rel pixel -> dst */

#ifdef BUILD_MMX
#define _op_blend_rel_p_dp_mmx _op_blend_p_dpan_mmx 
#define _op_blend_rel_pas_dp_mmx _op_blend_pas_dpan_mmx 
#define _op_blend_rel_pan_dp_mmx _op_blend_pan_dpan_mmx 

#define _op_blend_rel_p_dpan_mmx _op_blend_p_dpan_mmx
#define _op_blend_rel_pas_dpan_mmx _op_blend_pas_dpan_mmx
#define _op_blend_rel_pan_dpan_mmx _op_blend_pan_dpan_mmx

#define _op_blend_rel_p_dpas_mmx _op_blend_rel_p_dp_mmx
#define _op_blend_rel_pas_dpas_mmx _op_blend_rel_pas_dp_mmx
#define _op_blend_rel_pan_dpas_mmx _op_blend_rel_pan_dp_mmx

static void
init_blend_rel_pixel_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_blend_rel_p_dp_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_blend_rel_pas_dp_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_blend_rel_pan_dp_mmx;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_p_dpan_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pas_dpan_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pan_dpan_mmx;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_p_dpas_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pas_dpas_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pan_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
#define _op_blend_rel_pt_p_dp_mmx _op_blend_pt_p_dpan_mmx 
#define _op_blend_rel_pt_pas_dp_mmx _op_blend_pt_pas_dpan_mmx 
#define _op_blend_rel_pt_pan_dp_mmx _op_blend_pt_pan_dpan_mmx 

#define _op_blend_rel_pt_p_dpan_mmx _op_blend_pt_p_dpan_mmx
#define _op_blend_rel_pt_pas_dpan_mmx _op_blend_pt_pas_dpan_mmx
#define _op_blend_rel_pt_pan_dpan_mmx _op_blend_pt_pan_dpan_mmx

#define _op_blend_rel_pt_p_dpas_mmx _op_blend_rel_pt_p_dp_mmx
#define _op_blend_rel_pt_pas_dpas_mmx _op_blend_rel_pt_pas_dp_mmx
#define _op_blend_rel_pt_pan_dpas_mmx _op_blend_rel_pt_pan_dp_mmx

static void
init_blend_rel_pixel_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_p_dp_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_pas_dp_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_blend_rel_pt_pan_dp_mmx;

   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_p_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_pas_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_blend_rel_pt_pan_dpan_mmx;

   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pt_p_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pt_pas_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_blend_rel_pt_pan_dpas_mmx;
}
#endif
