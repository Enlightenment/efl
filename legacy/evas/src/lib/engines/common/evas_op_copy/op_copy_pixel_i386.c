
/* copy pixel --> dst */

#ifdef BUILD_MMX
static void
_op_copy_p_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l - 15;
   for (; d < e; d+=16, s+=16) {
      MOVE_16DWORDS_MMX(s, d);
   }
   e+=15;
   for (; d < e; d++, s++) {
      *d = *s;
   }
}

#define _op_copy_pan_dp_mmx _op_copy_p_dp_mmx
#define _op_copy_pas_dp_mmx _op_copy_p_dp_mmx

#define _op_copy_p_dpan_mmx _op_copy_p_dp_mmx
#define _op_copy_pan_dpan_mmx _op_copy_pan_dp_mmx
#define _op_copy_pas_dpan_mmx _op_copy_pas_dp_mmx

#define _op_copy_p_dpas_mmx _op_copy_p_dp_mmx
#define _op_copy_pan_dpas_mmx _op_copy_pan_dp_mmx
#define _op_copy_pas_dpas_mmx _op_copy_pas_dp_mmx

static void
init_copy_pixel_span_funcs_mmx(void)
{
   op_copy_span_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_copy_p_dp_mmx;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_copy_pan_dp_mmx;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_copy_pas_dp_mmx;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_p_dpan_mmx;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_pan_dpan_mmx;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_pas_dpan_mmx;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_p_dpas_mmx;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_pan_dpas_mmx;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_pas_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_copy_pt_p_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
      *d = s;
}

#define _op_copy_pt_pan_dp_mmx _op_copy_pt_p_dp_mmx
#define _op_copy_pt_pas_dp_mmx _op_copy_pt_p_dp_mmx

#define _op_copy_pt_p_dpan_mmx _op_copy_pt_p_dp_mmx
#define _op_copy_pt_pan_dpan_mmx _op_copy_pt_pan_dp_mmx
#define _op_copy_pt_pas_dpan_mmx _op_copy_pt_pas_dp_mmx

#define _op_copy_pt_p_dpas_mmx _op_copy_pt_p_dp_mmx
#define _op_copy_pt_pan_dpas_mmx _op_copy_pt_pan_dp_mmx
#define _op_copy_pt_pas_dpas_mmx _op_copy_pt_pas_dp_mmx

static void
init_copy_pixel_pt_funcs_mmx(void)
{
   op_copy_pt_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_copy_pt_p_dp_mmx;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_copy_pt_pan_dp_mmx;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_copy_pt_pas_dp_mmx;

   op_copy_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_pt_p_dpan_mmx;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_pt_pan_dpan_mmx;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_pt_pas_dpan_mmx;

   op_copy_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_pt_p_dpas_mmx;
   op_copy_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_pt_pan_dpas_mmx;
   op_copy_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_pt_pas_dpas_mmx;
}
#endif

/*-----*/

/* copy_rel pixel --> dst */

#ifdef BUILD_MMX
static void
_op_copy_rel_p_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(PIX_BLACK, mm6, mm0)
   movq_r2r(mm6, mm7);
   psllq_i2r(48, mm6);
   psrlq_i2r(16, mm7);
   for (; d < e; d++, s++) {
	MOV_P2R(*d, mm1, mm0)
	pand_r2r(mm6, mm1);
	MOV_P2R(*s, mm2, mm0)
	MUL4_SYM_R2R(mm2, mm1, mm6);
	pand_r2r(mm7, mm2);
	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
   }
}

static void
_op_copy_rel_pas_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(PIX_BLACK, mm6, mm0)
   movq_r2r(mm6, mm7);
   psllq_i2r(48, mm6);
   psrlq_i2r(16, mm7);
   for (; d < e; d++, s++) {
      switch(*s & 0xff000000)
	 {
	   case 0:
	     *d = *s;
	   break;
	   case 0xff000000:
	     MOV_P2R(*d, mm1, mm0)
	     pand_r2r(mm6, mm1);
	     MOV_P2R(*s, mm2, mm0)
	     pand_r2r(mm7, mm2);
	     paddw_r2r(mm2, mm1);
	     MOV_R2P(mm1, *d, mm0)
	   break;
	   default:
	     MOV_P2R(*d, mm1, mm0)
	     pand_r2r(mm6, mm1);
	     MOV_P2R(*s, mm2, mm0)
	     MUL4_SYM_R2R(mm2, mm1, mm6);
	     pand_r2r(mm7, mm2);
	     paddw_r2r(mm2, mm1);
	     MOV_R2P(mm1, *d, mm0)
	   break;
	 }
   }
}

static void
_op_copy_rel_pan_dp_mmx(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {
   DATA32 *e = d + l;
   pxor_r2r(mm0, mm0);
   MOV_P2R(PIX_BLACK, mm6, mm0)
   movq_r2r(mm6, mm7);
   psllq_i2r(48, mm6);
   psrlq_i2r(16, mm7);
   for (; d < e; d++, s++) {
	MOV_P2R(*d, mm1, mm0)
	pand_r2r(mm6, mm1);
	MOV_P2R(*s, mm2, mm0)
	pand_r2r(mm7, mm2);
	paddw_r2r(mm2, mm1);
	MOV_R2P(mm1, *d, mm0)
   }
}

#define _op_copy_rel_p_dpan_mmx _op_copy_p_dpan_mmx
#define _op_copy_rel_pan_dpan_mmx _op_copy_pan_dpan_mmx
#define _op_copy_rel_pas_dpan_mmx _op_copy_pas_dpan_mmx

#define _op_copy_rel_p_dpas_mmx _op_copy_rel_p_dp_mmx
#define _op_copy_rel_pan_dpas_mmx _op_copy_rel_pan_dp_mmx
#define _op_copy_rel_pas_dpas_mmx _op_copy_rel_pas_dp_mmx

static void
init_copy_rel_pixel_span_funcs_mmx(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_p_dp_mmx;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_pan_dp_mmx;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_pas_dp_mmx;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_p_dpan_mmx;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pan_dpan_mmx;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pas_dpan_mmx;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_p_dpas_mmx;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_pan_dpas_mmx;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_pas_dpas_mmx;
}
#endif

#ifdef BUILD_MMX
static void
_op_copy_rel_pt_p_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
   DATA32 sa = 1 + (s >> 24);
   s &= 0x00ffffff;
   pxor_r2r(mm0, mm0);
   MOV_P2R(s, mm2, mm0)
   MOV_A2R(sa, mm3)
   psllq_i2r(48, mm3);
   MOV_P2R(*d, mm1, mm0)
   MUL4_256_R2R(mm3, mm1)
   paddw_r2r(mm2, mm1);
   MOV_R2P(mm1, *d, mm0)
}

static void
_op_copy_rel_pt_pan_dp_mmx(DATA32 s, DATA8 m, DATA32 c, DATA32 *d) {
	*d = (*d & 0xff000000) + (s & 0x00ffffff);
}

#define _op_copy_rel_pt_pas_dp_mmx _op_copy_rel_pt_p_dp_mmx

#define _op_copy_rel_pt_p_dpan_mmx _op_copy_pt_p_dpan_mmx
#define _op_copy_rel_pt_pan_dpan_mmx _op_copy_pt_pan_dpan_mmx
#define _op_copy_rel_pt_pas_dpan_mmx _op_copy_pt_pas_dpan_mmx

#define _op_copy_rel_pt_p_dpas_mmx _op_copy_rel_pt_p_dp_mmx
#define _op_copy_rel_pt_pan_dpas_mmx _op_copy_rel_pt_pan_dp_mmx
#define _op_copy_rel_pt_pas_dpas_mmx _op_copy_rel_pt_pas_dp_mmx

static void
init_copy_rel_pixel_pt_funcs_mmx(void)
{
   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_pt_p_dp_mmx;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_pt_pan_dp_mmx;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_MMX] = _op_copy_rel_pt_pas_dp_mmx;

   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pt_p_dpan_mmx;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pt_pan_dpan_mmx;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_MMX] = _op_copy_rel_pt_pas_dpan_mmx;

   op_copy_rel_pt_funcs[SP][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_pt_p_dpas_mmx;
   op_copy_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_pt_pan_dpas_mmx;
   op_copy_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AS][CPU_MMX] = _op_copy_rel_pt_pas_dpas_mmx;
}
#endif
