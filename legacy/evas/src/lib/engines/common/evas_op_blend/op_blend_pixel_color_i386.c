
/* blend pixel x color --> dst */

#ifdef BUILD_MMX
static void
_op_blend_p_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)
   movq_r2r(mm7, mm4);
   psllq_i2r(48, mm4);
   MOV_P2R(c, mm2, mm0)
   while (d < e)
     {
	DATA32 da = *d >> 24;
	DATA32 sa;
	MOV_P2R(*s, mm3, mm0)
	MUL4_SYM_R2R(mm2, mm3, mm7)
	packuswb_r2r(mm0, mm3);

	MOV_RA2R(mm3, mm1)
	movd_r2m(mm3, sa);
	da = (_evas_pow_lut[((sa >> 16) & 0xff00) + da]) << 24;
	MOV_MA2R(da, mm5);
	psrlq_i2r(16, mm5);

	psllq_i2r(48, mm1);
	por_r2r(mm1, mm5);
	psrlw_i2r(1, mm5);

	MOV_P2R(*d, mm1, mm0)
	punpcklbw_r2r(mm0, mm3);
	por_r2r(mm4, mm3);
	BLEND_R2R(mm5, mm3, mm1, mm6)
	MOV_R2P(mm1, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_pas_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)
   movq_r2r(mm7, mm4);
   psllq_i2r(48, mm4);
   MOV_P2R(c, mm2, mm0)
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		break;
	    default:
	      {
		DATA32 da = *d >> 24;
		DATA32 sa;
		MOV_P2R(*s, mm3, mm0)
		MUL4_SYM_R2R(mm2, mm3, mm7)
		packuswb_r2r(mm0, mm3);

		MOV_RA2R(mm3, mm1)
		movd_r2m(mm3, sa);
		da = (_evas_pow_lut[((sa >> 16) & 0xff00) + da]) << 24;
		MOV_MA2R(da, mm5);
		psrlq_i2r(16, mm5);

		psllq_i2r(48, mm1);
		por_r2r(mm1, mm5);
		psrlw_i2r(1, mm5);

		MOV_P2R(*d, mm1, mm0)
		punpcklbw_r2r(mm0, mm3);
		por_r2r(mm4, mm3);
		BLEND_R2R(mm5, mm3, mm1, mm6)
		MOV_R2P(mm1, *d, mm0)
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_c_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l, ca = c & 0xff000000;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)
   MOV_MA2R(ca, mm4)
   psllq_i2r(48, mm4);
   ca >>= 16;
   c |= 0xff000000;
   MOV_P2R(c, mm2, mm0)
   while (d < e)
     {
	DATA32  da = (_evas_pow_lut[ca + (*d >> 24)]) << 24;
	MOV_MA2R(da, mm5);
	psrlq_i2r(16, mm5);
	por_r2r(mm4, mm5);
	psrlw_i2r(1, mm5);

	MOV_P2R(*s, mm3, mm0)
	MUL4_SYM_R2R(mm2, mm3, mm7)

	MOV_P2R(*d, mm1, mm0)
	BLEND_R2R(mm5, mm3, mm1, mm6)
	MOV_R2P(mm1, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_p_can_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)
   movq_r2r(mm7, mm4);
   psllq_i2r(48, mm4);
   MOV_P2R(c, mm2, mm0)
   while (d < e)
     {
	DATA32 da = (_evas_pow_lut[((*s >> 16) & 0xff00) + (*d >> 24)]) << 24;
	MOV_MA2R(da, mm5);
	psrlq_i2r(16, mm5);
	
	MOV_P2R(*s, mm3, mm0)
	MUL4_SYM_R2R(mm2, mm3, mm7)

	packuswb_r2r(mm0, mm3);
	MOV_RA2R(mm3, mm1)
	psllq_i2r(48, mm1);
	por_r2r(mm1, mm5);
	psrlw_i2r(1, mm5);

	punpcklbw_r2r(mm0, mm3);
	por_r2r(mm4, mm3);
	MOV_P2R(*d, mm1, mm0)
	BLEND_R2R(mm5, mm3, mm1, mm6)
	MOV_R2P(mm1, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_pas_can_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)
   movq_r2r(mm7, mm4);
   psllq_i2r(48, mm4);
   MOV_P2R(c, mm2, mm0)
   while (d < e)
     {
	switch(*s & 0xff000000)
	  {
	    case 0:
		break;
	    case 0xff000000:
		MOV_P2R(*s, mm1, mm0)
		MUL4_SYM_R2R(mm2, mm1, mm7)
		MOV_R2P(mm1, *d, mm0)
		break;
	    default:
	      {
		DATA32 da = (_evas_pow_lut[((*s >> 16) & 0xff00) + (*d >> 24)]) << 24;
		MOV_MA2R(da, mm5);
		psrlq_i2r(16, mm5);

		MOV_P2R(*s, mm3, mm0)
		MUL4_SYM_R2R(mm2, mm3, mm7)
		packuswb_r2r(mm0, mm3);

		MOV_RA2R(mm3, mm1)
		psllq_i2r(48, mm1);
		por_r2r(mm1, mm5);
		psrlw_i2r(1, mm5);

		punpcklbw_r2r(mm0, mm3);
		por_r2r(mm4, mm3);
		MOV_P2R(*d, mm1, mm0)
		BLEND_R2R(mm5, mm3, mm1, mm6)
		MOV_R2P(mm1, *d, mm0)
	      }
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_can_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(PIX_WHITE, mm6, mm0)
   MOV_P2R(c, mm2, mm0)
   while (d < e)
     {
	MOV_P2R(*s, mm1, mm0)
	MUL4_SYM_R2R(mm2, mm1, mm6)
	MOV_R2P(mm1, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_p_c_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32  *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);
   while (d < e)
     {
	MOV_P2R(*d, mm2, mm0)
	MOV_P2R(*s, mm1, mm0)
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm5, mm1);
	packuswb_r2r(mm0, mm1);

	MOV_RA2R(mm1, mm3)
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	punpcklbw_r2r(mm0, mm1);
	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_pas_c_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32  *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);
   while (d < e)
     {
	DATA32  a = *s >> 24;
	switch (a)
	  {
	   case 0: break;
	   case 255:
		MOV_P2R(*d, mm2, mm0)
		MOV_P2R(*s, mm1, mm0)
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		MOV_RA2R(mm1, mm3)
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);
		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	   default:
		MOV_P2R(*d, mm2, mm0)
		MOV_P2R(*s, mm1, mm0)
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		MOV_RA2R(mm1, mm3)
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);
		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_c_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32  *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);
   while (d < e)
     {
	MOV_P2R(*d, mm2, mm0)
	MOV_P2R(*s, mm1, mm0)
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm5, mm1);
	packuswb_r2r(mm0, mm1);

	MOV_RA2R(mm1, mm3)
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	punpcklbw_r2r(mm0, mm1);
	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_p_can_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);
   while (d < e)
     {
	MOV_P2R(*d, mm2, mm0)
	MOV_P2R(*s, mm1, mm0)
	psllw_i2r(1, mm1);
	paddw_r2r(mm6, mm1);
	pmulhw_r2r(mm5, mm1);
	packuswb_r2r(mm0, mm1);

	MOV_RA2R(mm1, mm3)
	psrlw_i2r(1, mm3);
	psrlq_i2r(16, mm3);

	punpcklbw_r2r(mm0, mm1);
	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_pas_can_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);
   while (d < e)
     {
	DATA32  a = *s >> 24;
	switch (a)
	  {
	   case 0:  break;
	   case 255:
	        a = (*d | 0x00ffffff) & *s;
		MOV_P2R(a, mm1, mm0)
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		MOV_R2P(mm1, *d, mm0)
		break;
	   default:
		MOV_P2R(*d, mm2, mm0)
		MOV_P2R(*s, mm1, mm0)
		psllw_i2r(1, mm1);
		paddw_r2r(mm6, mm1);
		pmulhw_r2r(mm5, mm1);
		packuswb_r2r(mm0, mm1);

		MOV_RA2R(mm1, mm3)
		psrlw_i2r(1, mm3);
		psrlq_i2r(16, mm3);

		punpcklbw_r2r(mm0, mm1);
		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_p_caa_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32  *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_MA2R(c, mm5)
   psrlw_i2r(1, mm5);
   psrlq_i2r(16, mm5);
   while (d < e)
     {
	DATA32  a = *s >> 24;
	MOV_A2R(a, mm3)
	psllw_i2r(1, mm3);
	paddw_r2r(mm6, mm3);
	pmulhw_r2r(mm5, mm3);
	packuswb_r2r(mm0, mm3);
	punpcklbw_r2r(mm3, mm3);
	psrlw_i2r(1, mm3);

	MOV_P2R(*d, mm2, mm0)
	MOV_P2R(*s, mm1, mm0)

	BLEND_R2R(mm3, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	s++;  d++;
     }
}

static void
_op_blend_pas_caa_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32  *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_MA2R(c, mm5)
   psrlw_i2r(1, mm5);
   psrlq_i2r(16, mm5);
   while (d < e)
     {
	DATA32  a = *s >> 24;
	switch (a)
	  {
	   case 0: break;
	   case 255:
		MOV_P2R(*d, mm2, mm0)
		a = *s & c;
		MOV_P2R(a, mm1, mm0)
		BLEND_R2R(mm5, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	   default:
		MOV_A2R(a, mm3)
		psllw_i2r(1, mm3);
		paddw_r2r(mm6, mm3);
		pmulhw_r2r(mm5, mm3);
		packuswb_r2r(mm0, mm3);
		punpcklbw_r2r(mm3, mm3);
		psrlw_i2r(1, mm3);

		MOV_P2R(*d, mm2, mm0)
		MOV_P2R(*s, mm1, mm0)
		BLEND_R2R(mm3, mm1, mm2, mm6)
		MOV_R2P(mm2, *d, mm0)
		break;
	  }
	s++;  d++;
     }
}

static void
_op_blend_pan_caa_dpan_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_MA2R(c, mm5)
   psrlw_i2r(1, mm5);
   psrlq_i2r(16, mm5);
   while (d < e)
     {
	DATA32  a = *s & c;
	MOV_P2R(a, mm1, mm0)
	MOV_P2R(*d, mm2, mm0)
	BLEND_R2R(mm5, mm1, mm2, mm6)
	MOV_R2P(mm2, *d, mm0)
	s++;  d++;
     }
}

#define _op_blend_p_caa_dp_mmx _op_blend_p_c_dp_mmx
#define _op_blend_pas_caa_dp_mmx _op_blend_pas_c_dp_mmx
#define _op_blend_pan_caa_dp_mmx _op_blend_pan_c_dp_mmx

#define _op_blend_pan_can_dpan_mmx _op_blend_pan_can_dp_mmx

#define _op_blend_p_c_dpas_mmx _op_blend_p_c_dp_mmx
#define _op_blend_pas_c_dpas_mmx _op_blend_pas_c_dp_mmx
#define _op_blend_pan_c_dpas_mmx _op_blend_pan_c_dp_mmx
#define _op_blend_p_can_dpas_mmx _op_blend_p_can_dp_mmx
#define _op_blend_pas_can_dpas_mmx _op_blend_pas_can_dp_mmx
#define _op_blend_pan_can_dpas_mmx _op_blend_pan_can_dp_mmx
#define _op_blend_p_caa_dpas_mmx _op_blend_p_caa_dp_mmx
#define _op_blend_pas_caa_dpas_mmx _op_blend_pas_caa_dp_mmx
#define _op_blend_pan_caa_dpas_mmx _op_blend_pan_caa_dp_mmx

static void
init_blend_pixel_color_span_funcs_mmx(void)
{
   op_blend_span_funcs[SP][SM_N][SC][DP][CPU_MMX] = _op_blend_p_c_dp_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP][CPU_MMX] = _op_blend_pas_c_dp_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP][CPU_MMX] = _op_blend_pan_c_dp_mmx;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_p_can_dp_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_pas_can_dp_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_pan_can_dp_mmx;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_p_caa_dp_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_pas_caa_dp_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_pan_caa_dp_mmx;

   op_blend_span_funcs[SP][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_p_c_dpan_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_pas_c_dpan_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_pan_c_dpan_mmx;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_p_can_dpan_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_pas_can_dpan_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_pan_can_dpan_mmx;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_p_caa_dpan_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_pas_caa_dpan_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_pan_caa_dpan_mmx;

   op_blend_span_funcs[SP][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_p_c_dpas_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_pas_c_dpas_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_pan_c_dpas_mmx;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_p_can_dpas_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_pas_can_dpas_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_pan_can_dpas_mmx;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_p_caa_dpas_mmx;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_pas_caa_dpas_mmx;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_pan_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_blend_pt_p_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   DATA32 da = *d >> 24;
   DATA32 sa;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)
   movq_r2r(mm7, mm4);
   psllq_i2r(48, mm4);

   MOV_P2R(c, mm2, mm0)
   MOV_P2R(s, mm3, mm0)
   MUL4_SYM_R2R(mm2, mm3, mm7)
   packuswb_r2r(mm0, mm3);

   MOV_RA2R(mm3, mm1)
   movd_r2m(mm3, sa);
   da = (_evas_pow_lut[((sa >> 16) & 0xff00) + da]) << 24;
   MOV_MA2R(da, mm5);
   psrlq_i2r(16, mm5);

   psllq_i2r(48, mm1);
   por_r2r(mm1, mm5);
   psrlw_i2r(1, mm5);

   MOV_P2R(*d, mm1, mm0)
   punpcklbw_r2r(mm0, mm3);
   por_r2r(mm4, mm3);
   BLEND_R2R(mm5, mm3, mm1, mm6)
   MOV_R2P(mm1, *d, mm0)
}

static void
_op_blend_pt_pan_c_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   DATA32  da = (_evas_pow_lut[((c >> 16) & 0xff00) + (*d >> 24)]) << 24;
   DATA32  ca = c & 0xff000000;
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)

   MOV_MA2R(ca, mm4)
   psllq_i2r(48, mm4);

   MOV_MA2R(da, mm5);
   psrlq_i2r(16, mm5);
   por_r2r(mm4, mm5);
   psrlw_i2r(1, mm5);

   c |= 0xff000000;
   MOV_P2R(c, mm2, mm0)
   MOV_P2R(s, mm3, mm0)
   MUL4_SYM_R2R(mm2, mm3, mm7)

   MOV_P2R(*d, mm1, mm0)
   BLEND_R2R(mm5, mm3, mm1, mm6)
   MOV_R2P(mm1, *d, mm0)
}

static void
_op_blend_pt_p_can_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   DATA32 da = (_evas_pow_lut[((s >> 16) & 0xff00) + (*d >> 24)]) << 24;
   MOV_MA2R(da, mm5);
   psrlq_i2r(16, mm5);
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   MOV_P2R(PIX_WHITE, mm7, mm0)
   movq_r2r(mm7, mm4);
   psllq_i2r(48, mm4);

   MOV_P2R(c, mm2, mm0)
   MOV_P2R(s, mm3, mm0)
   MUL4_SYM_R2R(mm2, mm3, mm7)
   packuswb_r2r(mm0, mm3);

   MOV_RA2R(mm3, mm1)
   psllq_i2r(48, mm1);
   por_r2r(mm1, mm5);
   psrlw_i2r(1, mm5);

   punpcklbw_r2r(mm0, mm3);
   por_r2r(mm4, mm3);
   MOV_P2R(*d, mm1, mm0)
   BLEND_R2R(mm5, mm3, mm1, mm6)
   MOV_R2P(mm1, *d, mm0)
}

static void
_op_blend_pt_pan_can_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   pxor_r2r(mm0, mm0);
   MOV_P2R(PIX_WHITE, mm7, mm0)
   MOV_P2R(c, mm2, mm0)
   MOV_P2R(s, mm1, mm0)
   MUL4_SYM_R2R(mm2, mm1, mm7)
   MOV_R2P(mm1, *d, mm0)
}

static void
_op_blend_pt_p_c_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);
   MOV_P2R(*d, mm2, mm0)
   MOV_P2R(s, mm1, mm0)
   psllw_i2r(1, mm1);
   paddw_r2r(mm6, mm1);
   pmulhw_r2r(mm5, mm1);
   packuswb_r2r(mm0, mm1);

   MOV_RA2R(mm1, mm3)
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);

   punpcklbw_r2r(mm0, mm1);
   BLEND_R2R(mm3, mm1, mm2, mm6)
   MOV_R2P(mm2, *d, mm0)
}

static void
_op_blend_pt_pan_c_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);
   MOV_P2R(*d, mm2, mm0)
   MOV_P2R(s, mm1, mm0)
   psllw_i2r(1, mm1);
   paddw_r2r(mm6, mm1);
   pmulhw_r2r(mm5, mm1);
   packuswb_r2r(mm0, mm1);

   MOV_RA2R(mm1, mm3)
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);

   punpcklbw_r2r(mm0, mm1);
   BLEND_R2R(mm3, mm1, mm2, mm6)
   MOV_R2P(mm2, *d, mm0)
}

static void
_op_blend_pt_p_can_dpan_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_ONE, mm6)
   movd_m2r(c, mm5);
   punpcklbw_r2r(mm5, mm5);
   psrlw_i2r(1, mm5);

   MOV_P2R(*d, mm2, mm0)
   MOV_P2R(s, mm1, mm0)
   psllw_i2r(1, mm1);
   paddw_r2r(mm6, mm1);
   pmulhw_r2r(mm5, mm1);
   packuswb_r2r(mm0, mm1);

   MOV_RA2R(mm1, mm3)
   psrlw_i2r(1, mm3);
   psrlq_i2r(16, mm3);

   punpcklbw_r2r(mm0, mm1);
   BLEND_R2R(mm3, mm1, mm2, mm6)
   MOV_R2P(mm2, *d, mm0)
}

#define _op_blend_pt_p_caa_dp_mmx _op_blend_pt_p_c_dp_mmx
#define _op_blend_pt_pan_caa_dp_mmx _op_blend_pt_pan_c_dp_mmx
#define _op_blend_pt_pas_c_dp_mmx _op_blend_pt_p_c_dp_mmx
#define _op_blend_pt_pas_can_dp_mmx _op_blend_pt_p_can_dp_mmx
#define _op_blend_pt_pas_caa_dp_mmx _op_blend_pt_p_caa_dp_mmx

#define _op_blend_pt_pas_c_dpan_mmx _op_blend_pt_p_c_dpan_mmx
#define _op_blend_pt_pas_can_dpan_mmx _op_blend_pt_p_can_dpan_mmx
#define _op_blend_pt_pan_can_dpan_mmx _op_blend_pt_pan_can_dp_mmx
#define _op_blend_pt_p_caa_dpan_mmx _op_blend_pt_p_c_dpan_mmx
#define _op_blend_pt_pas_caa_dpan_mmx _op_blend_pt_p_caa_dpan_mmx
#define _op_blend_pt_pan_caa_dpan_mmx _op_blend_pt_pan_c_dpan_mmx

#define _op_blend_pt_p_c_dpas_mmx _op_blend_pt_p_c_dp_mmx
#define _op_blend_pt_pas_c_dpas_mmx _op_blend_pt_pas_c_dp_mmx
#define _op_blend_pt_pan_c_dpas_mmx _op_blend_pt_pan_c_dp_mmx
#define _op_blend_pt_p_can_dpas_mmx _op_blend_pt_p_can_dp_mmx
#define _op_blend_pt_pas_can_dpas_mmx _op_blend_pt_pas_can_dp_mmx
#define _op_blend_pt_pan_can_dpas_mmx _op_blend_pt_pan_can_dp_mmx
#define _op_blend_pt_p_caa_dpas_mmx _op_blend_pt_p_caa_dp_mmx
#define _op_blend_pt_pas_caa_dpas_mmx _op_blend_pt_pas_caa_dp_mmx
#define _op_blend_pt_pan_caa_dpas_mmx _op_blend_pt_pan_caa_dp_mmx

static void
init_blend_pixel_color_pt_funcs_mmx(void)
{
   op_blend_pt_funcs[SP][SM_N][SC][DP][CPU_MMX] = _op_blend_pt_p_c_dp_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP][CPU_MMX] = _op_blend_pt_pas_c_dp_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP][CPU_MMX] = _op_blend_pt_pan_c_dp_mmx;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_pt_p_can_dp_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_pt_pas_can_dp_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_pt_pan_can_dp_mmx;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_pt_p_caa_dp_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_pt_pas_caa_dp_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_pt_pan_caa_dp_mmx;

   op_blend_pt_funcs[SP][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_pt_p_c_dpan_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_pt_pas_c_dpan_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_pt_pan_c_dpan_mmx;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_pt_p_can_dpan_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_pt_pas_can_dpan_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_pt_pan_can_dpan_mmx;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_pt_p_caa_dpan_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_pt_pas_caa_dpan_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_pt_pan_caa_dpan_mmx;

   op_blend_pt_funcs[SP][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_pt_p_c_dpas_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_pt_pas_c_dpas_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_pt_pan_c_dpas_mmx;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_pt_p_can_dpas_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_pt_pas_can_dpas_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_pt_pan_can_dpas_mmx;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_pt_p_caa_dpas_mmx;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_pt_pas_caa_dpas_mmx;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_pt_pan_caa_dpas_mmx;
}
#endif

/*-----*/

/* blend_rel pixel x color -> dst */

#ifdef BUILD_MMX
#define _op_blend_rel_p_c_dp_mmx _op_blend_p_c_dpan_mmx
#define _op_blend_rel_pas_c_dp_mmx _op_blend_pas_c_dpan_mmx
#define _op_blend_rel_pan_c_dp_mmx _op_blend_pan_c_dpan_mmx
#define _op_blend_rel_p_can_dp_mmx _op_blend_p_can_dpan_mmx
#define _op_blend_rel_pas_can_dp_mmx _op_blend_pas_can_dpan_mmx
#define _op_blend_rel_pan_can_dp_mmx _op_blend_pan_can_dpan_mmx
#define _op_blend_rel_p_caa_dp_mmx _op_blend_p_caa_dpan_mmx
#define _op_blend_rel_pas_caa_dp_mmx _op_blend_pas_caa_dpan_mmx
#define _op_blend_rel_pan_caa_dp_mmx _op_blend_pan_caa_dpan_mmx

#define _op_blend_rel_p_c_dpan_mmx _op_blend_p_c_dpan_mmx
#define _op_blend_rel_pas_c_dpan_mmx _op_blend_pas_c_dpan_mmx
#define _op_blend_rel_pan_c_dpan_mmx _op_blend_pan_c_dpan_mmx
#define _op_blend_rel_p_can_dpan_mmx _op_blend_p_can_dpan_mmx
#define _op_blend_rel_pas_can_dpan_mmx _op_blend_pas_can_dpan_mmx
#define _op_blend_rel_pan_can_dpan_mmx _op_blend_pan_can_dpan_mmx
#define _op_blend_rel_p_caa_dpan_mmx _op_blend_p_caa_dpan_mmx
#define _op_blend_rel_pas_caa_dpan_mmx _op_blend_pas_caa_dpan_mmx
#define _op_blend_rel_pan_caa_dpan_mmx _op_blend_pan_caa_dpan_mmx

#define _op_blend_rel_p_c_dpas_mmx _op_blend_rel_p_c_dp_mmx
#define _op_blend_rel_pas_c_dpas_mmx _op_blend_rel_pas_c_dp_mmx
#define _op_blend_rel_pan_c_dpas_mmx _op_blend_rel_pan_c_dp_mmx
#define _op_blend_rel_p_can_dpas_mmx _op_blend_rel_p_can_dp_mmx
#define _op_blend_rel_pas_can_dpas_mmx _op_blend_rel_pas_can_dp_mmx
#define _op_blend_rel_pan_can_dpas_mmx _op_blend_rel_pan_can_dp_mmx
#define _op_blend_rel_p_caa_dpas_mmx _op_blend_rel_p_caa_dp_mmx
#define _op_blend_rel_pas_caa_dpas_mmx _op_blend_rel_pas_caa_dp_mmx
#define _op_blend_rel_pan_caa_dpas_mmx _op_blend_rel_pan_caa_dp_mmx

static void
init_blend_rel_pixel_color_span_funcs_mmx(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_p_c_dp_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_pas_c_dp_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_pan_c_dp_mmx;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_rel_p_can_dp_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_rel_pas_can_dp_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_rel_pan_can_dp_mmx;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_p_caa_dp_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_pas_caa_dp_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_pan_caa_dp_mmx;

   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_p_c_dpan_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_pas_c_dpan_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_pan_c_dpan_mmx;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_p_can_dpan_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_pas_can_dpan_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_pan_can_dpan_mmx;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_p_caa_dpan_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pas_caa_dpan_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pan_caa_dpan_mmx;

   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_p_c_dpas_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_pas_c_dpas_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_pan_c_dpas_mmx;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_p_can_dpas_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_pas_can_dpas_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_pan_can_dpas_mmx;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_p_caa_dpas_mmx;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_pas_caa_dpas_mmx;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_pan_caa_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
#define _op_blend_rel_pt_p_c_dp_mmx _op_blend_pt_p_c_dpan_mmx
#define _op_blend_rel_pt_pas_c_dp_mmx _op_blend_pt_pas_c_dpan_mmx
#define _op_blend_rel_pt_pan_c_dp_mmx _op_blend_pt_pan_c_dpan_mmx
#define _op_blend_rel_pt_p_can_dp_mmx _op_blend_pt_p_can_dpan_mmx
#define _op_blend_rel_pt_pas_can_dp_mmx _op_blend_pt_pas_can_dpan_mmx
#define _op_blend_rel_pt_pan_can_dp_mmx _op_blend_pt_pan_can_dpan_mmx
#define _op_blend_rel_pt_p_caa_dp_mmx _op_blend_pt_p_caa_dpan_mmx
#define _op_blend_rel_pt_pas_caa_dp_mmx _op_blend_pt_pas_caa_dpan_mmx
#define _op_blend_rel_pt_pan_caa_dp_mmx _op_blend_pt_pan_caa_dpan_mmx

#define _op_blend_rel_pt_p_c_dpan_mmx _op_blend_pt_p_c_dpan_mmx
#define _op_blend_rel_pt_pas_c_dpan_mmx _op_blend_pt_pas_c_dpan_mmx
#define _op_blend_rel_pt_pan_c_dpan_mmx _op_blend_pt_pan_c_dpan_mmx
#define _op_blend_rel_pt_p_can_dpan_mmx _op_blend_pt_p_can_dpan_mmx
#define _op_blend_rel_pt_pas_can_dpan_mmx _op_blend_pt_pas_can_dpan_mmx
#define _op_blend_rel_pt_pan_can_dpan_mmx _op_blend_pt_pan_can_dpan_mmx
#define _op_blend_rel_pt_p_caa_dpan_mmx _op_blend_pt_p_caa_dpan_mmx
#define _op_blend_rel_pt_pas_caa_dpan_mmx _op_blend_pt_pas_caa_dpan_mmx
#define _op_blend_rel_pt_pan_caa_dpan_mmx _op_blend_pt_pan_caa_dpan_mmx

#define _op_blend_rel_pt_p_c_dpas_mmx _op_blend_rel_pt_p_c_dp_mmx
#define _op_blend_rel_pt_pas_c_dpas_mmx _op_blend_rel_pt_pas_c_dp_mmx
#define _op_blend_rel_pt_pan_c_dpas_mmx _op_blend_rel_pt_pan_c_dp_mmx
#define _op_blend_rel_pt_p_can_dpas_mmx _op_blend_rel_pt_p_can_dp_mmx
#define _op_blend_rel_pt_pas_can_dpas_mmx _op_blend_rel_pt_pas_can_dp_mmx
#define _op_blend_rel_pt_pan_can_dpas_mmx _op_blend_rel_pt_pan_can_dp_mmx
#define _op_blend_rel_pt_p_caa_dpas_mmx _op_blend_rel_pt_p_caa_dp_mmx
#define _op_blend_rel_pt_pas_caa_dpas_mmx _op_blend_rel_pt_pas_caa_dp_mmx
#define _op_blend_rel_pt_pan_caa_dpas_mmx _op_blend_rel_pt_pan_caa_dp_mmx

static void
init_blend_rel_pixel_color_pt_funcs_mmx(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_pt_p_c_dp_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_pt_pas_c_dp_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP][CPU_MMX] = _op_blend_rel_pt_pan_c_dp_mmx;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_rel_pt_p_can_dp_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_rel_pt_pas_can_dp_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_MMX] = _op_blend_rel_pt_pan_can_dp_mmx;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_pt_p_caa_dp_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_pt_pas_caa_dp_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_MMX] = _op_blend_rel_pt_pan_caa_dp_mmx;

   op_blend_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_pt_p_c_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_pt_pas_c_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_MMX] = _op_blend_rel_pt_pan_c_dpan_mmx;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_pt_p_can_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_pt_pas_can_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_MMX] = _op_blend_rel_pt_pan_can_dpan_mmx;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pt_p_caa_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pt_pas_caa_dpan_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_MMX] = _op_blend_rel_pt_pan_caa_dpan_mmx;

   op_blend_rel_pt_funcs[SP][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_pt_p_c_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_pt_pas_c_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP_AS][CPU_MMX] = _op_blend_rel_pt_pan_c_dpas_mmx;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_pt_p_can_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_pt_pas_can_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AS][CPU_MMX] = _op_blend_rel_pt_pan_can_dpas_mmx;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_pt_p_caa_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_pt_pas_caa_dpas_mmx;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AS][CPU_MMX] = _op_blend_rel_pt_pan_caa_dpas_mmx;
}
#endif
