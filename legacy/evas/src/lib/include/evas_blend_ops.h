#ifndef EVAS_BLEND_OPS_H
#define EVAS_BLEND_OPS_H

#if defined BUILD_MMX || defined BUILD_SSE
#include "evas_mmx.h"
#endif

/* src pixel flags: */

/* pixels none */
#define SP_N 0
/* pixels (argb default) */
#define SP 1
/* pixels are rgb (ie. alphas == 255) */
#define SP_AN 2
/* pixels alpha are sparse */
#define SP_AS 3
/* src pixels flags count */
#define SP_LAST 4

/* src mask flags: */

/* mask none */
#define SM_N 0
/* mask (alpha) */
#define SM 1
/* mask alphas are 'trivial - ie. only 0 or 255 */
#define SM_AT 2
/* mask alphas are sparse */
#define SM_AS 3
/* src mask flags count */
#define SM_LAST 4

/* src color flags: */

/* color is 0xffffffff */
#define SC_N 0
/* color (argb default) */
#define SC 1
/* color is rgb (ie. 0xffrrggbb) */
#define SC_AN 2
/* color is 'alpha' (ie. 0xaaffffff) */
#define SC_AA 3
/* src color flags count */
#define SC_LAST 4

/* dst pixels flags: */

/* pixels (argb default) */
#define DP  0
/* pixels are rgb (ie. alphas == 255) */
#define DP_AN  1
/* pixels alpha sparse */
#define DP_AS 2
/* dst pixels flags count */
#define DP_LAST 3

/* dst flags count */
#define DP_LAST 3

/* cpu types flags */

/* none, bad news */
#define CPU_N  0
/* cpu C */
#define CPU_C  1
/* cpu MMX */
#define CPU_MMX 2
/* cpu SSE */
#define CPU_SSE 3
/* cpu SSE2 */
#define CPU_SSE2 4
/* cpu flags count */
#define CPU_LAST 5


/* some useful constants */

extern const DATA32 PIX_WHITE;
extern const DATA32 PIX_BLACK;
//extern const DATA32 PIX_HALF;
extern const DATA32 ALPHA_256;
extern const DATA32 ALPHA_ONE;

extern const DATA16 _evas_const_c1[4];

extern const DATA8 _evas_pow_lut[65536];

/* some useful C macros */

#define MUL4_256(a, r, g, b, c) \
 ( (((((c) >> 8) & 0xff0000) * (a)) & 0xff000000) + \
   (((((c) & 0xff0000) * (r)) >> 8) & 0xff0000) + \
   (((((c) & 0xff00) * (g)) >> 8) & 0xff00) + \
   ((((c) & 0xff) * (b)) >> 8) )

#define MUL3_256(r, g, b, c) \
 ( (((((c) & 0xff0000) * (r)) >> 8) & 0xff0000) + \
   (((((c) & 0xff00) * (g)) >> 8) & 0xff00) + \
   ((((c) & 0xff) * (b)) >> 8) )

#define MUL_256(a, c) \
 ( (((((c) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + \
   (((((c) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff) )

#define MUL4_SYM(x, y) \
 ( ((((((x) >> 16) & 0xff00) * (((y) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) + \
   ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff00) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

#define MUL3_SYM(x, y) \
 ( ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff00) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

#define MUL_SYM(a, x) \
 ( (((((x) >> 8) & 0x00ff00ff) * (a) + 0xff00ff) & 0xff00ff00) + \
   (((((x) & 0x00ff00ff) * (a) + 0xff00ff) >> 8) & 0x00ff00ff) )

#define MUL_A_256(a, c) \
 ( ((((c) >> 8) & 0x00ff0000) * (a)) & 0xff000000 )

#define MUL_A_SYM(a, c) \
 ( (((((c) >> 8) & 0x00ff0000) * (a)) + 0x00ff0000) & 0xff000000 )

#define INTERP_256(a, c0, c1) \
 ( (((((((c0) >> 8) & 0xff00ff) - (((c1) >> 8) & 0xff00ff)) * (a)) \
   + ((c1) & 0xff00ff00)) & 0xff00ff00) + \
   (((((((c0) & 0xff00ff) - ((c1) & 0xff00ff)) * (a)) >> 8) \
   + ((c1) & 0xff00ff)) & 0xff00ff) )

#define INTERP_RGB_256(a, c0, c1) \
 ( (((((((c0) >> 8) & 0xff) - (((c1) >> 8) & 0xff)) * (a)) \
   + ((c1) & 0xff00)) & 0xff00) + \
   (((((((c0) & 0xff00ff) - ((c1) & 0xff00ff)) * (a)) >> 8) \
   + ((c1) & 0xff00ff)) & 0xff00ff) )

#define INTERP_A_256(a, c0, c1) \
 ( (((((((c0) >> 8) & 0xff0000) - (((c1) >> 8) & 0xff0000)) * (a)) \
   + ((c1) & 0xff000000)) & 0xff000000) )

#define BLEND_RGB_256(a, c0, c1) \
 ( (((((((c0) >> 8) & 0xff) - (((c1) >> 8) & 0xff)) * (a)) \
   + ((c1) & 0xff00ff00)) & 0xff00ff00) + \
   (((((((c0) & 0xff00ff) - ((c1) & 0xff00ff)) * (a)) >> 8) \
   + ((c1) & 0xff00ff)) & 0xff00ff) )

#define BLEND_ARGB_256(a, aa, c0, c1) \
 ( ((((0xff0000 - (((c1) >> 8) & 0xff0000)) * (a)) \
   + ((c1) & 0xff000000)) & 0xff000000) + \
   (((((((c0) >> 8) & 0xff) - (((c1) >> 8) & 0xff)) * (aa)) \
   + ((c1) & 0xff00)) & 0xff00) + \
   (((((((c0) & 0xff00ff) - ((c1) & 0xff00ff)) * (aa)) >> 8) \
   + ((c1) & 0xff00ff)) & 0xff00ff) )

#define ADD_REL(s, d, res) \
   { \
	DATA32  rb; \
	int     t, g; \
\
	rb = ((s) & 0xff00ff) + ((d) & 0xff00ff); \
	t = ((rb & 0xffff0000) >> 8) - (((d) >> 16) & 0xff00); \
	t = ((t & (~(t >> 8))) & 0xff00) << 8; \
\
	g = (rb & 0xffff) - ((d) >> 24); \
	rb -= t + ((g & (~(g >> 8))) & 0xff); \
\
	g = ((s) & 0xff00) + ((d) & 0xff00); \
	t = g - (((d) >> 16) & 0xff00); \
	g -= ((t & (~(t >> 8))) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + g; \
   }

#define MUL_ADD_REL_SYM(a, s, d, res) \
   { \
	DATA32  rb; \
	int     t, g; \
\
	rb = (((((a)*((s) & 0xff00ff)) + 0xff00ff) >> 8) & 0xff00ff) + ((d) & 0xff00ff); \
\
	t = ((rb & 0xffff0000) >> 8) - (((d) >> 16) & 0xff00); \
	t = ((t & (~(t >> 8))) & 0xff00) << 8; \
\
	g = (rb & 0xffff) - ((d) >> 24); \
	rb -= t + ((g & (~(g >> 8))) & 0xff); \
\
	g = (((((a)*((s) & 0xff00)) + 0xff00) >> 8) & 0xff00) + ((d) & 0xff00); \
	t = g - (((d) >> 16) & 0xff00); \
	g -= ((t & (~(t >> 8))) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + g; \
   }

#define MUL_ADD_REL_256(a, s, d, res) \
   { \
	DATA32  rb; \
	int     t, g; \
\
	rb = ((((a)*((s) & 0xff00ff)) >> 8) & 0xff00ff) + ((d) & 0xff00ff); \
	t = ((rb & 0xffff0000) >> 8) - (((d) >> 16) & 0xff00); \
	t = ((t & (~(t >> 8))) & 0xff00) << 8; \
\
	g = (rb & 0xffff) - ((d) >> 24); \
	rb -= t + ((g & (~(g >> 8))) & 0xff); \
\
	g = ((((a)*((s) & 0xff00)) >> 8) & 0xff00) + ((d) & 0xff00); \
	t = g - (((d) >> 16) & 0xff00); \
	g -= ((t & (~(t >> 8))) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + g; \
   }

#define ADD(s, d, res) \
   { \
	DATA32  rb, g; \
\
	rb = ((s) & 0xff00ff) + ((d) & 0xff00ff); \
	rb = (rb | (0x1000100 - ((rb >> 8) & 0xff00ff))) & 0xff00ff; \
\
	g = ((s) & 0xff00) + ((d) & 0xff00); \
	res = 0xff000000 + rb + ((g | (0x10000 - ((g >> 8) & 0xff00))) & 0xff00); \
   }

#define MUL_ADD_SYM(a, s, d, res) \
   { \
	DATA32  rb, g; \
\
	rb = (((((a)*((s) & 0xff00ff)) + 0xff00ff) >> 8) & 0xff00ff) + ((d) & 0xff00ff); \
	rb = (rb | (0x1000100 - ((rb >> 8) & 0xff00ff))) & 0xff00ff; \
	g = (((((a)*((s) & 0xff00)) + 0xff00) >> 8) & 0xff00) + ((d) & 0xff00); \
	res = 0xff000000 + rb + ((g | (0x10000 - ((g >> 8) & 0xff00))) & 0xff00); \
   }

#define MUL_ADD_256(a, s, d, res) \
   { \
	DATA32  rb, g; \
\
	rb = ((((a)*((s) & 0xff00ff)) >> 8) & 0xff00ff) + ((d) & 0xff00ff); \
	rb = (rb | (0x1000100 - ((rb >> 8) & 0xff00ff))) & 0xff00ff; \
	g = ((((a)*((s) & 0xff00)) >> 8) & 0xff00) + ((d) & 0xff00); \
	res = 0xff000000 + rb + ((g | (0x10000 - ((g >> 8) & 0xff00))) & 0xff00); \
   }

#define MUL2_ADD_256(a0, a1, s0, s1, res) \
   { \
	DATA32  rb, g; \
\
	rb = ((((a0)*((s0) & 0xff00ff)) >> 8) & 0xff00ff) + \
	     ((((a1)*((s1) & 0xff00ff)) >> 8) & 0xff00ff); \
	rb = (rb | (0x1000100 - ((rb >> 8) & 0xff00ff))) & 0xff00ff; \
\
	g = ((((a0)*((s0) & 0xff00)) >> 8) & 0xff00) + \
	    ((((a1)*((s1) & 0xff00)) >> 8) & 0xff00); \
	rb = rb + ((g | (0x10000 - ((g >> 8) & 0xff00))) & 0xff00); \
\
	g = ((s0) >> 24) + ((s1) >> 24); \
	g = ((g | (0x100 - ((g >> 8) & 0xff))) & 0xff) << 24; \
	res = g + rb; \
   }


#define SUB_REL(s, d, res) \
   { \
	int  rb, g; \
\
	g = ((d) & 0xff) - ((s) & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = (((d) & 0xff0000) >> 8) - (((s) & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((d) & 0xff00) - ((s) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define MUL_SUB_REL_SYM(a, s, d, res) \
   { \
	DATA32  t; \
	int     rb, g; \
\
	t = (((((a)*((s) & 0xff00ff)) + 0xff00ff) >> 8) & 0xff00ff); \
	g = ((d) & 0xff) - (t & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = (((d) & 0xff0000) >> 8) - ((t & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((d) & 0xff00) - (((((a)*((s) & 0xff00)) + 0xff00) >> 8) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define MUL_SUB_REL_256(a, s, d, res) \
   { \
	DATA32  t; \
	int     rb, g; \
\
	t = ((((a)*((s) & 0xff00ff)) >> 8) & 0xff00ff); \
	g = ((d) & 0xff) - (t & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = (((d) & 0xff0000) >> 8) - ((t & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((d) & 0xff00) - ((((a)*((s) & 0xff00)) >> 8) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define SUB(s, d, res) \
   { \
	int  rb, g; \
\
	g = ((d) & 0xff) - ((s) & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = (((d) & 0xff0000) >> 8) - (((s) & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((d) & 0xff00) - ((s) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define MUL_SUB_SYM(a, s, d, res) \
   { \
	DATA32  t; \
	int     rb, g; \
\
	t = (((((a)*((s) & 0xff00ff)) + 0xff00ff) >> 8) & 0xff00ff); \
	g = ((d) & 0xff) - (t & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = (((d) & 0xff0000) >> 8) - ((t & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((d) & 0xff00) - (((((a)*((s) & 0xff00)) + 0xff00) >> 8) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define MUL_SUB_REV_SYM(a, s, d, res) \
   { \
	DATA32  t; \
	int     rb, g; \
\
	t = (((((a)*((s) & 0xff00ff)) + 0xff00ff) >> 8) & 0xff00ff); \
	g = (t & 0xff) - ((d) & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = ((t & 0xff0000) >> 8) - (((d) & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = (((((a)*((s) & 0xff00)) + 0xff00) >> 8) & 0xff00) - ((d) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define MUL_SUB_256(a, s, d, res) \
   { \
	DATA32  t; \
	int     rb, g; \
\
	t = ((((a)*((s) & 0xff00ff)) >> 8) & 0xff00ff); \
	g = ((d) & 0xff) - (t & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = (((d) & 0xff0000) >> 8) - ((t & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((d) & 0xff00) - ((((a)*((s) & 0xff00)) >> 8) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define MUL_SUB_REV_256(a, s, d, res) \
   { \
	DATA32  t; \
	int     rb, g; \
\
	t = ((((a)*((s) & 0xff00ff)) >> 8) & 0xff00ff); \
	g = (t & 0xff) - ((d) & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = ((t & 0xff0000) >> 8) - (((d) & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((((a)*((s) & 0xff00)) >> 8) & 0xff00) - ((d) & 0xff00); \
\
	res = ((d) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define MUL2_SUB_256(a0, a1, s0, s1, res) \
   { \
	DATA32  t0, t1; \
	int     rb, g; \
\
	t1 = ((((a1)*((s1) & 0xff00ff)) >> 8) & 0xff00ff); \
	t0 = ((((a0)*((s0) & 0xff00ff)) >> 8) & 0xff00ff); \
\
	g = (t1 & 0xff) - (t0 & 0xff); \
	g = ((g & (~(g >> 8))) & 0xff); \
\
	rb = ((t1 & 0xff0000) >> 8) - ((t0 & 0xff0000) >> 8); \
	rb = (((rb & (~(rb >> 8))) & 0xff00) << 8) + g; \
\
	g = ((((a1)*((s1) & 0xff00)) >> 8) & 0xff00) - \
	    ((((a0)*((s0) & 0xff00)) >> 8) & 0xff00); \
\
	res = ((s1) & 0xff000000) + rb + ((g & (~(g >> 8))) & 0xff00); \
   }

#define SUB_A(s, d, res) \
   { \
	int  _a = ((d) >> 24) - ((s) >> 24); \
\
	res = ((d) & 0x00ffffff) + (((_a & (~(_a >> 8))) & 0xff) << 24); \
   }


/* some useful MMX macros */

#ifdef BUILD_MMX
#define MOV_A2R(a, mma) \
	movd_m2r(a, mma); \
	punpcklwd_r2r(mma, mma); \
	punpckldq_r2r(mma, mma);

#define MOV_P2R(c, mmc, mmz) \
	movd_m2r(c, mmc); \
	punpcklbw_r2r(mmz, mmc);

#define MOV_R2P(mmc, c, mmz) \
	packuswb_r2r(mmz, mmc); \
	movd_r2m(mmc, c);

#define MUL4_256_R2R(mmx, mmy) \
	pmullw_r2r(mmx, mmy); \
	psrlw_i2r(8, mmy);

#define MUL4_SYM_R2R(mmx, mmy, mm255) \
	pmullw_r2r(mmx, mmy); \
	paddw_r2r(mm255, mmy); \
	psrlw_i2r(8, mmy);

#define MOV_RA2R(mmx, mma) \
	movq_r2r(mmx, mma); \
	punpcklbw_r2r(mma, mma); \
	punpckhwd_r2r(mma, mma); \
	punpckhdq_r2r(mma, mma);

#define MOV_MA2R(a, mma) \
	movd_m2r(a, mma); \
	punpcklbw_r2r(mma, mma); \
	punpckhwd_r2r(mma, mma); \
	punpckhdq_r2r(mma, mma);

#define INTERP_256_R2R(mma, mmx, mmy) \
	psubusw_r2r(mmy, mmx); \
	pmullw_r2r(mma, mmx); \
	psrlw_i2r(8, mmx); \
	paddw_r2r(mmx, mmy);

#define BLEND_R2R(mma, mmx, mmy, mm_1) \
	psubw_r2r(mmy, mmx); \
	psllw_i2r(1, mmx); \
	paddw_r2r(mm_1, mmx); \
	pmulhw_r2r(mma, mmx); \
	paddw_r2r(mmx, mmy);
#endif

#endif
