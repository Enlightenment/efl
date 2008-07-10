/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_convert_yuv.h"

#if defined BUILD_MMX || defined BUILD_SSE
# include "evas_mmx.h"
#endif

#if defined HAVE_ALTIVEC_H
# include <altivec.h>
#ifdef CONFIG_DARWIN
#define AVV(x...) (x)
#else
#define AVV(x...) {x}
#endif

#endif

#ifdef BUILD_CONVERT_YUV

static void _evas_yuv_init         (void);
static void _evas_yv12torgb_sse    (unsigned char **yuv, unsigned char *rgb, int w, int h);
static void _evas_yv12torgb_mmx    (unsigned char **yuv, unsigned char *rgb, int w, int h);
static void _evas_yv12torgb_altivec(unsigned char **yuv, unsigned char *rgb, int w, int h);
static void _evas_yv12torgb_raster (unsigned char **yuv, unsigned char *rgb, int w, int h);
static void _evas_yv12torgb_diz    (unsigned char **yuv, unsigned char *rgb, int w, int h);

#define CRV    104595
#define CBU    132251
#define CGU    25624
#define CGV    53280
#define YMUL   76283
#define OFF    32768
#define BITRES 16

/* calculation float resolution in bits */
/* ie RES = 6 is 10.6 fixed point */
/*    RES = 8 is 8.8 fixed point */
/*    RES = 4 is 12.4 fixed point */
/* NB: going above 6 will lead to overflow... :( */
#define RES    6

#define RZ(i)  (i >> (BITRES - RES))
#define FOUR(i) {i, i, i, i}

#if defined BUILD_MMX || defined BUILD_SSE
__attribute__ ((aligned (8))) const volatile unsigned short _const_crvcrv[4] = FOUR(RZ(CRV));
__attribute__ ((aligned (8))) const volatile unsigned short _const_cbucbu[4] = FOUR(RZ(CBU));
__attribute__ ((aligned (8))) const volatile unsigned short _const_cgucgu[4] = FOUR(RZ(CGU));
__attribute__ ((aligned (8))) const volatile unsigned short _const_cgvcgv[4] = FOUR(RZ(CGV));
__attribute__ ((aligned (8))) const volatile unsigned short _const_ymul  [4] = FOUR(RZ(YMUL));
__attribute__ ((aligned (8))) const volatile unsigned short _const_128   [4] = FOUR(128);
__attribute__ ((aligned (8))) const volatile unsigned short _const_32    [4] = FOUR(RZ(OFF));
__attribute__ ((aligned (8))) const volatile unsigned short _const_16    [4] = FOUR(16);
__attribute__ ((aligned (8))) const volatile unsigned short _const_ff    [4] = FOUR(-1);

#define CONST_CRVCRV *_const_crvcrv
#define CONST_CBUCBU *_const_cbucbu
#define CONST_CGUCGU *_const_cgucgu
#define CONST_CGVCGV *_const_cgvcgv
#define CONST_YMUL   *_const_ymul
#define CONST_128    *_const_128
#define CONST_32     *_const_32
#define CONST_16     *_const_16
#define CONST_FF     *_const_ff

/* for C non aligned cleanup */
const int _crv = RZ(CRV);   /* 1.596 */
const int _cbu = RZ(CBU);   /* 2.018 */
const int _cgu = RZ(CGU);   /* 0.391 */
const int _cgv = RZ(CGV);   /* 0.813 */

#endif

#ifdef BUILD_ALTIVEC
#ifdef __VEC__
const vector unsigned short res     = AVV(RES);
const vector signed short crv       = AVV(RZ(CRV));
const vector signed short cbu       = AVV(RZ(CBU));
const vector signed short cgu       = AVV(RZ(CGU));
const vector signed short cgv       = AVV(RZ(CGV));
const vector signed short ymul      = AVV(RZ(YMUL));
const vector signed short c128      = AVV(128);
const vector signed short c32       = AVV(RZ(OFF));
const vector signed short c16       = AVV(16);
const vector unsigned char zero     = AVV(0);
const vector signed short maxchar   = AVV(255);
const vector unsigned char pickrg1  = AVV(0, 0x1, 0x11, 0,
					  0, 0x3, 0x13, 0,
					  0, 0x5, 0x15, 0,
					  0, 0x7, 0x17, 0);
const vector unsigned char pickrg2  = AVV(0, 0x9, 0x19, 0,
					  0, 0xb, 0x1b, 0,
					  0, 0xd, 0x1d, 0,
					  0, 0xf, 0x1f, 0);
const vector unsigned char pickrgb1 = AVV(0x3, 0x1, 0x2, 0x11,
					  0x7, 0x5, 0x6, 0x13,
					  0xb, 0x9, 0xa, 0x15,
					  0xf, 0xd, 0xe, 0x17);
const vector unsigned char pickrgb2 = AVV(0x3, 0x1, 0x2, 0x19,
					  0x7, 0x5, 0x6, 0x1b,
					  0xb, 0x9, 0xa, 0x1d,
					  0xf, 0xd, 0xe, 0x1f);
#endif
#endif

#ifdef BUILD_C

/* shortcut speedup lookup-tables */
static short _v1164[256];
static short _v1596[256];
static short _v813[256];
static short _v391[256];
static short _v2018[256];

static unsigned char _clip_lut[1024];
#define LUT_CLIP(i) ((_clip_lut+384)[(i)])

#define CMP_CLIP(i) ((i&256)? (~(i>>10)) : i);

#endif

void
evas_common_convert_yuv_420p_601_rgba(DATA8 **src, DATA8 *dst, int w, int h)
{
   int mmx, sse, sse2;

#if defined BUILD_MMX || defined BUILD_SSE
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
#endif
#ifndef BUILD_SSE
   sse = 0;
   sse2 = 0;
#endif
#ifndef BUILD_MMX
   mmx = 0;
#endif
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
     _evas_yv12torgb_sse(src, dst, w, h);
   else if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     _evas_yv12torgb_mmx(src, dst, w, h);
#ifdef BUILD_ALTIVEC
   if (evas_common_cpu_has_feature(CPU_FEATURE_ALTIVEC))
     _evas_yv12torgb_altivec(src, dst, w, h);
#endif
   else
     {
#ifdef BUILD_C
	static int initted = 0;

	if (!initted) _evas_yuv_init();
	initted = 1;
	/* FIXME: diz may be faster sometimes */
	_evas_yv12torgb_raster(src, dst, w, h);
#endif
     }
}

/* Thanks to Diz for this code. i've munged it a little and turned it into */
/* inline macros. I tried beating it with a different algorithm using MMX */
/* but failed. So here we are. This is the fastest YUV->RGB i know of for */
/* x86. It has an issue that it doesnt convert colours accurately so the */
/* image looks a little "yellowy". This is a result of only 10.6 fixed point */
/* resolution as opposed to 16.16 in the C code. This could be fixed by */
/* processing half the number of pixels per cycle and going up to 32bits */
/* per element during compute, but it would all but negate the speedup */
/* from mmx I think :( It might be possible to use SSE and SSE2 here, but */
/* I haven't tried yet. Let's see. */

/* NB: XviD has almost the same code in it's assembly YV12->RGB code. same */
/* algorithm, same constants, same all over actually, except it actually */
/* does a few extra memory accesses that this one doesn't, so in theory */
/* this code should be faster. In the end it's all just an mmx version of */
/* the reference implimentation done with fixed point math */

static void
_evas_yv12torgb_sse(unsigned char **yuv, unsigned char *rgb, int w, int h)
{
#ifdef BUILD_SSE
   int xx, yy;
   register unsigned char *yp1, *up, *vp;
   unsigned char *dp1;

   /* destination pointers */
   dp1 = rgb;

   for (yy = 0; yy < h; yy++)
     {
	/* plane pointers */
	yp1 = yuv[yy];
	up = yuv[h + (yy / 2)];
	vp = yuv[h + (h / 2) + (yy / 2)];
	for (xx = 0; xx < (w - 7); xx += 8)
	  {
	     movd_m2r(*up, mm3);
	     movd_m2r(*vp, mm2);
	     movq_m2r(*yp1, mm0);

	     pxor_r2r(mm7, mm7);
	     punpcklbw_r2r(mm7, mm2);
	     punpcklbw_r2r(mm7, mm3);

	     movq_r2r(mm0, mm1);
	     psrlw_i2r(8, mm0);
	     psllw_i2r(8, mm1);
	     psrlw_i2r(8, mm1);

	     movq_m2r(CONST_16, mm4);
	     psubsw_r2r(mm4, mm0);
	     psubsw_r2r(mm4, mm1);

	     movq_m2r(CONST_128, mm5);
	     psubsw_r2r(mm5, mm2);
	     psubsw_r2r(mm5, mm3);

	     movq_m2r(CONST_YMUL, mm4);
	     pmullw_r2r(mm4, mm0);
	     pmullw_r2r(mm4, mm1);

	     movq_m2r(CONST_CRVCRV, mm7);
	     pmullw_r2r(mm3, mm7);
	     movq_m2r(CONST_CBUCBU, mm6);
	     pmullw_r2r(mm2, mm6);
	     movq_m2r(CONST_CGUCGU, mm5);
	     pmullw_r2r(mm2, mm5);
	     movq_m2r(CONST_CGVCGV, mm4);
	     pmullw_r2r(mm3, mm4);

	     movq_r2r(mm0, mm2);
	     paddsw_r2r(mm7, mm2);
	     paddsw_r2r(mm1, mm7);

	     psraw_i2r(RES, mm2);
	     psraw_i2r(RES, mm7);
	     packuswb_r2r(mm7, mm2);

	     pxor_r2r(mm7, mm7);
	     movq_r2r(mm2, mm3);
	     punpckhbw_r2r(mm7, mm2);
	     punpcklbw_r2r(mm3, mm7);
	     por_r2r(mm7, mm2);

	     movq_r2r(mm0, mm3);
	     psubsw_r2r(mm5, mm3);
	     psubsw_r2r(mm4, mm3);
	     paddsw_m2r(CONST_32, mm3);

	     movq_r2r(mm1, mm7);
	     psubsw_r2r(mm5, mm7);
	     psubsw_r2r(mm4, mm7);
	     paddsw_m2r(CONST_32, mm7);

	     psraw_i2r(RES, mm3);
	     psraw_i2r(RES, mm7);
	     packuswb_r2r(mm7, mm3);

	     pxor_r2r(mm7, mm7);
	     movq_r2r(mm3, mm4);
	     punpckhbw_r2r(mm7, mm3);
	     punpcklbw_r2r(mm4, mm7);
	     por_r2r(mm7, mm3);

	     movq_m2r(CONST_32, mm4);
	     paddsw_r2r(mm6, mm0);
	     paddsw_r2r(mm6, mm1);
	     paddsw_r2r(mm4, mm0);
	     paddsw_r2r(mm4, mm1);
	     psraw_i2r(RES, mm0);
	     psraw_i2r(RES, mm1);
	     packuswb_r2r(mm1, mm0);

	     pxor_r2r(mm7, mm7);
	     movq_r2r(mm0, mm5);
	     punpckhbw_r2r(mm7, mm0);
	     punpcklbw_r2r(mm5, mm7);
	     por_r2r(mm7, mm0);

	     movq_m2r(CONST_FF, mm1);
	     movq_r2r(mm0, mm5);
	     movq_r2r(mm3, mm6);
	     movq_r2r(mm2, mm7);
	     punpckhbw_r2r(mm3, mm2);
	     punpcklbw_r2r(mm6, mm7);
	     punpckhbw_r2r(mm1, mm0);
	     punpcklbw_r2r(mm1, mm5);

	     movq_r2r(mm7, mm1);
	     punpckhwd_r2r(mm5, mm7);
	     punpcklwd_r2r(mm5, mm1);

	     movq_r2r(mm2, mm4);
	     punpckhwd_r2r(mm0, mm2);
	     punpcklwd_r2r(mm0, mm4);

	     movntq_r2m(mm1, *(dp1));
	     movntq_r2m(mm7, *(dp1 + 8));
	     movntq_r2m(mm4, *(dp1 + 16));
	     movntq_r2m(mm2, *(dp1 + 24));

	     yp1 += 8;
	     up += 4;
	     vp += 4;
	     dp1 += 8 * 4;
	  }
	/* cleanup pixles that arent a multiple of 8 pixels wide */
	if (xx < w)
	  {
	     int y, u, v, r, g, b;

	     for (; xx < w; xx += 2)
	       {
		  u = (*up++) - 128;
		  v = (*vp++) - 128;

		  A_VAL(dp1) = 0xff;
		  y = RZ(YMUL) * ((*yp1++) - 16);
		  r = (y + (_crv * v)) >> RES;
		  r = LUT_CLIP(r);
		  R_VAL(dp1) = r;
		  g = (y - (_cgu * u) - (_cgv * v) + RZ(OFF)) >> RES;
		  g = LUT_CLIP(g);
		  G_VAL(dp1) = g;
		  b = (y + (_cbu * u) + RZ(OFF)) >> RES;
		  b = LUT_CLIP(b);
		  B_VAL(dp1) = b;

		  dp1 += 4;

		  A_VAL(dp1) = 0xff;
		  y = RZ(YMUL) * ((*yp1++) - 16);
		  r = (y + (_crv * v)) >> RES;
		  r = LUT_CLIP(r);
		  R_VAL(dp1) = r;
		  g = (y - (_cgu * u) - (_cgv * v) + RZ(OFF)) >> RES;
		  g = LUT_CLIP(g);
		  G_VAL(dp1) = g;
		  b = (y + (_cbu * u) + RZ(OFF)) >> RES;
		  b = LUT_CLIP(b);
		  B_VAL(dp1) = b;

		  dp1 += 4;
	       }
	  }
     }
   emms();
#else
   _evas_yv12torgb_mmx(yuv, rgb, w, h);
#endif
}

static void
_evas_yv12torgb_mmx(unsigned char **yuv, unsigned char *rgb, int w, int h)
{
#ifdef BUILD_MMX
   int xx, yy;
   register unsigned char *yp1, *up, *vp;
   unsigned char *dp1;

   /* destination pointers */
   dp1 = rgb;

   for (yy = 0; yy < h; yy++)
     {
	/* plane pointers */
	yp1 = yuv[yy];
	up = yuv[h + (yy / 2)];
	vp = yuv[h + (h / 2) + (yy / 2)];
	for (xx = 0; xx < (w - 7); xx += 8)
	  {
	     movd_m2r(*up, mm3);
	     movd_m2r(*vp, mm2);
	     movq_m2r(*yp1, mm0);

	     pxor_r2r(mm7, mm7);
	     punpcklbw_r2r(mm7, mm2);
	     punpcklbw_r2r(mm7, mm3);

	     movq_r2r(mm0, mm1);
	     psrlw_i2r(8, mm0);
	     psllw_i2r(8, mm1);
	     psrlw_i2r(8, mm1);

	     movq_m2r(CONST_16, mm4);
	     psubsw_r2r(mm4, mm0);
	     psubsw_r2r(mm4, mm1);

	     movq_m2r(CONST_128, mm5);
	     psubsw_r2r(mm5, mm2);
	     psubsw_r2r(mm5, mm3);

	     movq_m2r(CONST_YMUL, mm4);
	     pmullw_r2r(mm4, mm0);
	     pmullw_r2r(mm4, mm1);

	     movq_m2r(CONST_CRVCRV, mm7);
	     pmullw_r2r(mm3, mm7);
	     movq_m2r(CONST_CBUCBU, mm6);
	     pmullw_r2r(mm2, mm6);
	     movq_m2r(CONST_CGUCGU, mm5);
	     pmullw_r2r(mm2, mm5);
	     movq_m2r(CONST_CGVCGV, mm4);
	     pmullw_r2r(mm3, mm4);

	     movq_r2r(mm0, mm2);
	     paddsw_r2r(mm7, mm2);
	     paddsw_r2r(mm1, mm7);

	     psraw_i2r(RES, mm2);
	     psraw_i2r(RES, mm7);
	     packuswb_r2r(mm7, mm2);

	     pxor_r2r(mm7, mm7);
	     movq_r2r(mm2, mm3);
	     punpckhbw_r2r(mm7, mm2);
	     punpcklbw_r2r(mm3, mm7);
	     por_r2r(mm7, mm2);

	     movq_r2r(mm0, mm3);
	     psubsw_r2r(mm5, mm3);
	     psubsw_r2r(mm4, mm3);
	     paddsw_m2r(CONST_32, mm3);

	     movq_r2r(mm1, mm7);
	     psubsw_r2r(mm5, mm7);
	     psubsw_r2r(mm4, mm7);
	     paddsw_m2r(CONST_32, mm7);

	     psraw_i2r(RES, mm3);
	     psraw_i2r(RES, mm7);
	     packuswb_r2r(mm7, mm3);

	     pxor_r2r(mm7, mm7);
	     movq_r2r(mm3, mm4);
	     punpckhbw_r2r(mm7, mm3);
	     punpcklbw_r2r(mm4, mm7);
	     por_r2r(mm7, mm3);

	     movq_m2r(CONST_32, mm4);
	     paddsw_r2r(mm6, mm0);
	     paddsw_r2r(mm6, mm1);
	     paddsw_r2r(mm4, mm0);
	     paddsw_r2r(mm4, mm1);
	     psraw_i2r(RES, mm0);
	     psraw_i2r(RES, mm1);
	     packuswb_r2r(mm1, mm0);

	     pxor_r2r(mm7, mm7);
	     movq_r2r(mm0, mm5);
	     punpckhbw_r2r(mm7, mm0);
	     punpcklbw_r2r(mm5, mm7);
	     por_r2r(mm7, mm0);

	     movq_m2r(CONST_FF, mm1);
	     movq_r2r(mm0, mm5);
	     movq_r2r(mm3, mm6);
	     movq_r2r(mm2, mm7);
	     punpckhbw_r2r(mm3, mm2);
	     punpcklbw_r2r(mm6, mm7);
	     punpckhbw_r2r(mm1, mm0);
	     punpcklbw_r2r(mm1, mm5);

	     movq_r2r(mm7, mm1);
	     punpckhwd_r2r(mm5, mm7);
	     punpcklwd_r2r(mm5, mm1);

	     movq_r2r(mm2, mm4);
	     punpckhwd_r2r(mm0, mm2);
	     punpcklwd_r2r(mm0, mm4);

	     movq_r2m(mm1, *(dp1));
	     movq_r2m(mm7, *(dp1 + 8));
	     movq_r2m(mm4, *(dp1 + 16));
	     movq_r2m(mm2, *(dp1 + 24));

	     yp1 += 8;
	     up += 4;
	     vp += 4;
	     dp1 += 8 * 4;
	  }
	/* cleanup pixles that arent a multiple of 8 pixels wide */
	if (xx < w)
	  {
	     int y, u, v, r, g, b;

	     for (; xx < w; xx += 2)
	       {
		  u = (*up++) - 128;
		  v = (*vp++) - 128;

		  A_VAL(dp1) = 0xff;
		  y = RZ(YMUL) * ((*yp1++) - 16);
		  r = (y + (_crv * v)) >> RES;
		  r = LUT_CLIP(r);
		  R_VAL(dp1) = r;
		  g = (y - (_cgu * u) - (_cgv * v) + RZ(OFF)) >> RES;
		  g = LUT_CLIP(g);
		  G_VAL(dp1) = g;
		  b = (y + (_cbu * u) + RZ(OFF)) >> RES;
		  b = LUT_CLIP(b);
		  B_VAL(dp1) = b;

		  dp1 += 4;

		  A_VAL(dp1) = 0xff;
		  y = RZ(YMUL) * ((*yp1++) - 16);
		  r = (y + (_crv * v)) >> RES;
		  r = LUT_CLIP(r);
		  R_VAL(dp1) = r;
		  g = (y - (_cgu * u) - (_cgv * v) + RZ(OFF)) >> RES;
		  g = LUT_CLIP(g);
		  G_VAL(dp1) = g;
		  b = (y + (_cbu * u) + RZ(OFF)) >> RES;
		  b = LUT_CLIP(b);
		  B_VAL(dp1) = b;

		  dp1 += 4;
	       }
	  }
     }
   emms();
#else
   _evas_yv12torgb_raster(yuv, rgb, w, h);
#endif
}

static void
_evas_yv12torgb_altivec(unsigned char **yuv, unsigned char *rgb, int w, int h)
{
#ifdef BUILD_ALTIVEC
#ifdef __VEC__
   int xx, yy;
   int w2, h2;
   unsigned char *yp1, *yp2, *up, *vp;
   unsigned char *dp1, *dp2;
   vector signed short y, u, v;
   vector signed short r, g, b;
   vector signed short tmp1, tmp2, tmp3;
   vector unsigned char yperm, uperm, vperm, rgb1, rgb2;
   vector unsigned char alpha;

   /* handy halved w & h */
   w2 = w / 2;
   h2 = h / 2;
   /* plane pointers */
   yp1 = yuv;
   yp2 = yuv + w;
   up = yuv + (w * h);
   vp = up + (w2 * h2);
   /* destination pointers */
   dp1 = rgb;
   dp2 = rgb + (w * 4);

   alpha = vec_mergeh((vector unsigned char)AVV(255), zero);
   alpha = (vector unsigned char)vec_mergeh((vector unsigned short)alpha,
					    (vector unsigned short)zero);

   for (yy = 0; yy < h2; yy++)
     {
	for (xx = 0; xx < w2; xx += 4)
	  {
/* Cycles */
	     /*
	      * Load 4 y and 4 u & v pixels for the 8x2 pixel block.
	      */
/* 3 */      tmp3 = (vector signed short)vec_lde(0, (unsigned int *)yp1);
/* 3 */      tmp1 = (vector signed short)vec_lde(0, (unsigned int *)up);
/* 3 */      tmp2 = (vector signed short)vec_lde(0, (unsigned int *)vp);

	     /* Prepare for aligning the data in their vectors */
/* 3 */      yperm = vec_lvsl(0, yp1);
/* 3 */      uperm = vec_lvsl(0, up);
/* 3 */      vperm = vec_lvsl(0, vp);
	     yp1 += 4;

	     /* Save y and load the next 4 y pixels for a total of 8 */
/* 2 */      y = vec_perm(tmp3, tmp3, yperm);
/* 3 */      tmp3 = (vector signed short)vec_lde(0, (unsigned int *)yp1);

	     /* Setup and calculate the 4 u pixels */
/* 2 */      tmp1 = vec_perm(tmp1, tmp1, uperm);
/* 2 */      tmp2 = vec_perm(tmp2, tmp2, vperm);

	     /* Avoid dependancy stalls on yperm and calculate the 4 u values */
/* 3 */      yperm = vec_lvsr(12, yp1);
/* 1 */      tmp1 = (vector signed short)vec_mergeh((vector unsigned char)tmp1,
						    (vector unsigned char)tmp1);
/* 1 */      u = (vector signed short)vec_mergeh(zero,
						 (vector unsigned char)tmp1);

/* 1 */      u = vec_sub(u, c128);
/* 2 */      tmp3 = vec_perm(tmp3, tmp3, yperm);

	     /* Setup and calculate the 4 v values */
/* 1 */      tmp2 = (vector signed short)vec_mergeh((vector unsigned char)tmp2,
						    (vector unsigned char)tmp2);
/* 1 */      v = (vector signed short)vec_mergeh(zero,
						 (vector unsigned char)tmp2);
/* 4 */      tmp2 = vec_mladd(cgu, u, (vector signed short)zero);
/* 1 */      v = vec_sub(v, c128);

	     /* Move the data into y and start loading the next 4 pixels */
/* 1 */      y = (vector signed short)vec_mergeh(zero,
						 (vector unsigned char)y);
/* 1 */      tmp3 = (vector signed short)vec_mergeh(zero,
						    (vector unsigned char)tmp3);
/* 1 */      y = vec_or(y, tmp3);

	     /* Finish calculating y */
/* 1 */      y = vec_sub(y, c16);
/* 4 */      y = vec_mladd(ymul, y, (vector signed short)zero);

	     /* Perform non-dependant multiplies first. */
/* 4 */      tmp1 = vec_mladd(crv, v, y);
/* 4 */      tmp2 = vec_mladd(cgv, v, tmp2);
/* 4 */      tmp3 = vec_mladd(cbu, u, y);

	     /* Calculate rgb values */
/* 1 */	     r = vec_sra(tmp1, res);

/* 1 */	     tmp2 = vec_sub(y, tmp2);
/* 1 */      tmp2 = vec_add(tmp2, c32);
/* 1 */      g = vec_sra(tmp2, res);

/* 1 */	     tmp3 = vec_add(tmp3, c32);
/* 1 */	     b = vec_sra(tmp3, res);

	     /* Bound to 0 <= x <= 255 */
/* 1 */	     r = vec_min(r, maxchar);
/* 1 */	     g = vec_min(g, maxchar);
/* 1 */	     b = vec_min(b, maxchar);
/* 1 */	     r = vec_max(r, (vector signed short)zero);
/* 1 */	     g = vec_max(g, (vector signed short)zero);
/* 1 */	     b = vec_max(b, (vector signed short)zero);

	     /* Combine r, g and b. */
/* 2 */	     rgb1 = vec_perm((vector unsigned char)r, (vector unsigned char)g,
			     pickrg1);
/* 2 */	     rgb2 = vec_perm((vector unsigned char)r, (vector unsigned char)g,
			    pickrg2);

/* 2 */	     rgb1 = vec_perm(rgb1, (vector unsigned char)b, pickrgb1);
/* 2 */	     rgb2 = vec_perm(rgb2, (vector unsigned char)b, pickrgb2);

/* 1 */      rgb1 = vec_or(alpha, rgb1);
/* 1 */      rgb2 = vec_or(alpha, rgb2);

/* 3 */	     vec_stl(rgb1, 0, dp1);
	     dp1 += 16;
/* 3 */	     vec_stl(rgb2, 0, dp1);

	     /*
	      * Begin the second row calculations
	      */

	     /*
	      * Load 4 y pixels for the 8x2 pixel block.
	      */
/* 3 */      yperm = vec_lvsl(0, yp2);
/* 3 */      tmp3 = (vector signed short)vec_lde(0, (unsigned int *)yp2);
	     yp2 += 4;

	     /* Save y and load the next 4 y pixels for a total of 8 */
/* 2 */      y = vec_perm(tmp3, tmp3, yperm);
/* 3 */      yperm = vec_lvsr(12, yp2);
/* 3 */      tmp3 = (vector signed short)vec_lde(0, (unsigned int *)yp2);
/* 1 */      y = (vector signed short)vec_mergeh(zero,
						 (vector unsigned char)y);

	     /* Avoid dependancy stalls on yperm */
/* 2 */      tmp3 = vec_perm(tmp3, tmp3, yperm);
/* 1 */      tmp3 = (vector signed short)vec_mergeh(zero,
						    (vector unsigned char)tmp3);
/* 1 */      y = vec_or(y, tmp3);

	     /* Start the calculation for g */
/* 4 */      tmp2 = vec_mladd(cgu, u, (vector signed short)zero);

	     /* Finish calculating y */
/* 1 */      y = vec_sub(y, c16);
/* 4 */      y = vec_mladd(ymul, y, (vector signed short)zero);

	     /* Perform non-dependant multiplies first. */
/* 4 */      tmp2 = vec_mladd(cgv, v, tmp2);
/* 4 */      tmp1 = vec_mladd(crv, v, y);
/* 4 */      tmp3 = vec_mladd(cbu, u, y);

	     /* Calculate rgb values */
/* 1 */	     r = vec_sra(tmp1, res);

/* 1 */	     tmp2 = vec_sub(y, tmp2);
/* 1 */      tmp2 = vec_add(tmp2, c32);
/* 1 */      g = vec_sra(tmp2, res);

/* 1 */	     tmp3 = vec_add(tmp3, c32);
/* 1 */	     b = vec_sra(tmp3, res);

	     /* Bound to 0 <= x <= 255 */
/* 1 */	     r = vec_min(r, maxchar);
/* 1 */	     g = vec_min(g, maxchar);
/* 1 */	     b = vec_min(b, maxchar);
/* 1 */	     r = vec_max(r, (vector signed short)zero);
/* 1 */	     g = vec_max(g, (vector signed short)zero);
/* 1 */	     b = vec_max(b, (vector signed short)zero);

	     /* Combine r, g and b. */
/* 2 */	     rgb1 = vec_perm((vector unsigned char)r, (vector unsigned char)g,
			    pickrg1);
/* 2 */	     rgb2 = vec_perm((vector unsigned char)r, (vector unsigned char)g,
			    pickrg2);

/* 2 */	     rgb1 = vec_perm(rgb1, (vector unsigned char)b, pickrgb1);
/* 2 */	     rgb2 = vec_perm(rgb2, (vector unsigned char)b, pickrgb2);

/* 1 */      rgb1 = vec_or(alpha, rgb1);
/* 1 */      rgb2 = vec_or(alpha, rgb2);

/* 3 */	     vec_stl(rgb1, 0, dp2);
	     dp2 += 16;
/* 3 */	     vec_stl(rgb2, 0, dp2);

	     /* Increment the YUV data pointers to the next set of pixels. */
	     yp1 += 4;
	     yp2 += 4;
	     up += 4;
	     vp += 4;

	     /* Move the destination pointers to the next set of pixels. */
	     dp1 += 16;
	     dp2 += 16;
	  }

	/* jump down one line since we are doing 2 at once */
	yp1 += w;
	yp2 += w;
	dp1 += (w * 4);
	dp2 += (w * 4);
     }
#endif
#else
   _evas_yv12torgb_diz(yuv, rgb, w, h);
#endif
}

static void
_evas_yuv_init(void)
{
#ifdef BUILD_C
   int i;

   for (i = 0; i < 256; i++)
     {
	_v1164[i] = (int)(((float)(i - 16 )) * 1.164);

	_v1596[i] = (int)(((float)(i - 128)) * 1.596);
	_v813[i]  = (int)(((float)(i - 128)) * 0.813);

	_v391[i]  = (int)(((float)(i - 128)) * 0.391);
	_v2018[i] = (int)(((float)(i - 128)) * 2.018);
     }

   for (i = -384; i < 640; i++)
     {
	_clip_lut[i+384] = i < 0 ? 0 : (i > 255) ? 255 : i;
     }
#endif
}

static void
_evas_yv12torgb_diz(unsigned char **yuv, unsigned char *rgb, int w, int h)
{
#ifdef BUILD_C
   int xx, yy;
   int y, u, v, r, g, b;
   unsigned char *yp1, *yp2, *up, *vp;
   unsigned char *dp1, *dp2;
   int crv, cbu, cgu, cgv;

   /* destination pointers */
   dp1 = rgb;
   dp2 = rgb + (w * 4);

   crv = CRV;   /* 1.596 */
   cbu = CBU;   /* 2.018 */
   cgu = CGU;   /* 0.391 */
   cgv = CGU;   /* 0.813 */

   for (yy = 0; yy < h; yy += 2)
     {
	/* plane pointers */
	yp1 = yuv[yy];
	yp2 = yuv[yy + 1];
	up = yuv[h + (yy / 2)];
	vp = yuv[h + (h / 2) + (yy / 2)];
	for (xx = 0; xx < w; xx += 2)
	  {
	     /* collect u & v for 2x2 pixel block */
	     u = (*up++) - 128;
	     v = (*vp++) - 128;

	     /* do the top 2 pixels of the 2x2 block whcih shared u & v */
	     /* yuv to rgb */
	     A_VAL(dp1) = 0xff;
	     y = YMUL * ((*yp1++) - 16);
	     r = (y + (crv * v)) >> 16;
	     r = LUT_CLIP(r);
	     R_VAL(dp1) = r;
	     g = (y - (cgu * u) - (cgv * v) + OFF) >>16;
	     g = LUT_CLIP(g);
	     G_VAL(dp1) = g;
	     b = (y + (cbu * u) + OFF) >> 16;
	     b = LUT_CLIP(b);
	     B_VAL(dp1) = b;

	     dp1 += 4;

	     /* yuv to rgb */
	     A_VAL(dp1) = 0xff;
	     y = YMUL * ((*yp1++) - 16);
	     r = (y + (crv * v)) >> 16;
	     r = LUT_CLIP(r);
	     R_VAL(dp1) = r;
	     g = (y - (cgu * u) - (cgv * v) + OFF) >>16;
	     g = LUT_CLIP(g);
	     G_VAL(dp1) = g;
	     b = (y + (cbu * u) + OFF) >> 16;
	     b = LUT_CLIP(b);
	     B_VAL(dp1) = b;

	     dp1 += 4;

	     /* do the bottom 2 pixels */
	     /* yuv to rgb */
	     A_VAL(dp2) = 0xff;
	     y = YMUL * ((*yp2++) - 16);
	     r = (y + (crv * v)) >> 16;
	     r = LUT_CLIP(r);
	     R_VAL(dp2) = r;
	     g = (y - (cgu * u) - (cgv * v) + OFF) >>16;
	     g = LUT_CLIP(g);
	     G_VAL(dp2) = g;
	     b = (y + (cbu * u) + OFF) >> 16;
	     b = LUT_CLIP(b);
	     B_VAL(dp2) = b;

	     dp2 += 4;

	     /* yuv to rgb */
	     A_VAL(dp2) = 0xff;
	     y = YMUL * ((*yp2++) - 16);
	     r = (y + (crv * v)) >> 16;
	     r = LUT_CLIP(r);
	     R_VAL(dp2) = r;
	     g = (y - (cgu * u) - (cgv * v) + OFF) >>16;
	     g = LUT_CLIP(g);
	     G_VAL(dp2) = g;
	     b = (y + (cbu * u) + OFF) >> 16;
	     b = LUT_CLIP(b);
	     B_VAL(dp2) = b;

	     dp2 += 4;
	  }
	/* jump down one line since we are doing 2 at once */
	dp1 += (w * 4);
	dp2 += (w * 4);
     }
#endif
}

static void
_evas_yv12torgb_raster(unsigned char **yuv, unsigned char *rgb, int w, int h)
{
#ifdef BUILD_C
   int xx, yy;
   int y, u, v, r, g, b;
   unsigned char *yp1, *yp2, *up, *vp;
   unsigned char *dp1, *dp2;

   /* destination pointers */
   dp1 = rgb;
   dp2 = rgb + (w * 4);

   for (yy = 0; yy < h; yy += 2)
     {
	/* plane pointers */
	yp1 = yuv[yy];
	yp2 = yuv[yy + 1];
	up = yuv[h + (yy / 2)];
	vp = yuv[h + (h / 2) + (yy / 2)];
	for (xx = 0; xx < w; xx += 2)
	  {
	     int vmu;

	     /* collect u & v for 2x2 pixel block */
	     u = *up++;
	     v = *vp++;

	     /* save lookups */
	     vmu = _v813[v] + _v391[u];
	     u = _v2018[u];
	     v = _v1596[v];

             /* do the top 2 pixels of the 2x2 block whcih shared u & v */
	     /* yuv to rgb */
	     y = _v1164[*yp1++];
	     r = y + v;
	     r = LUT_CLIP(r);
	     g = y -  vmu;
	     g = LUT_CLIP(g);
	     b = y + u;
	     b = LUT_CLIP(b);

	     A_VAL(dp1) = 0xff;
	     R_VAL(dp1) = r;
	     G_VAL(dp1) = g;
	     B_VAL(dp1) = b;

	     dp1 += 4;

	     /* yuv to rgb */
	     y = _v1164[*yp1++];
	     r = y + v;
	     r = LUT_CLIP(r);
	     g = y -  vmu;
	     g = LUT_CLIP(g);
	     b = y + u;
	     b = LUT_CLIP(b);

	     A_VAL(dp1) = 0xff;
	     R_VAL(dp1) = r;
	     G_VAL(dp1) = g;
	     B_VAL(dp1) = b;

	     dp1 += 4;

	     /* do the bottom 2 pixels */
	     /* yuv to rgb */
	     y = _v1164[*yp2++];
	     r = y + v;
	     r = LUT_CLIP(r);
	     g = y -  vmu;
	     g = LUT_CLIP(g);
	     b = y + u;
	     b = LUT_CLIP(b);

	     A_VAL(dp2) = 0xff;
	     R_VAL(dp2) = r;
	     G_VAL(dp2) = g;
	     B_VAL(dp2) = b;

	     dp2 += 4;

	     /* yuv to rgb */
	     y = _v1164[*yp2++];
	     r = y + v;
	     r = LUT_CLIP(r);
	     g = y -  vmu;
	     g = LUT_CLIP(g);
	     b = y + u;
	     b = LUT_CLIP(b);

	     A_VAL(dp2) = 0xff;
	     R_VAL(dp2) = r;
	     G_VAL(dp2) = g;
	     B_VAL(dp2) = b;
	     
	     dp2 += 4;
	  }
	/* jump down one line since we are doing 2 at once */
	dp1 += (w * 4);
	dp2 += (w * 4);
     }
#endif
}

#endif
