#ifndef EVAS_BLEND_OPS_H
#define EVAS_BLEND_OPS_H

#if defined BUILD_MMX || defined BUILD_SSE
#include "evas_mmx.h"
#endif

#if defined BUILD_SSE3
#include <immintrin.h>
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
/* color is 'alpha' (ie. 0xaaaaaaaa) */
#define SC_AA 3
/* src color flags count */
#define SC_LAST 4

/* dst pixels flags: */

/* pixels (argb default) */
#define DP  0
/* pixels are rgb (ie. alphas == 255) */
#define DP_AN  1
/* dst pixels flags count */
#define DP_LAST 2

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
#define CPU_NEON 5
/* CPU SSE3 */
#define CPU_SSE3 6
/* cpu flags count */
#define CPU_LAST 7


/* some useful constants */

extern const DATA32 ALPHA_255;
extern const DATA32 ALPHA_256;

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
	punpckhwd_r2r(mma, mma); \
	punpckhdq_r2r(mma, mma);

#define MOV_PA2R(c, mma) \
	movd_m2r(c, mma); \
	punpcklbw_r2r(mma, mma); \
	punpckhwd_r2r(mma, mma); \
	punpckhdq_r2r(mma, mma);

#define INTERP_256_R2R(mma, mmx, mmy, mm255) \
	psubw_r2r(mmy, mmx); \
	pmullw_r2r(mma, mmx); \
	psrlw_i2r(8, mmx); \
	paddw_r2r(mmx, mmy); \
	pand_r2r(mm255, mmy);

#endif


/* some useful SSE3 inline functions */

#ifdef BUILD_SSE3

static __m128i GA_MASK_SSE3;
static __m128i RB_MASK_SSE3;
static __m128i SYM4_MASK_SSE3;
static __m128i RGB_MASK_SSE3;
static __m128i A_MASK_SSE3;

static __m128i ALPHA_SSE3;

static always_inline __m128i
mul_256_sse3(__m128i a, __m128i c) {

   /* prepare alpha for word multiplication */
   __m128i a_l = a;
   __m128i a_h = a;
   a_l = _mm_unpacklo_epi16(a_l, a_l);
   a_h = _mm_unpackhi_epi16(a_h, a_h);
   __m128i a0 = (__m128i) _mm_shuffle_ps( (__m128)a_l, (__m128)a_h, 0x88);

   /* first half of calc */
   __m128i c0 = c;
   c0 = _mm_srli_epi32(c0, 8);
   c0 = _mm_and_si128(GA_MASK_SSE3, c0);
   c0 = _mm_mullo_epi16(a0, c0);
   c0 = _mm_and_si128(RB_MASK_SSE3, c0);

   /* second half of calc */
   __m128i c1 = c;
   c1 = _mm_and_si128(GA_MASK_SSE3, c1);
   c1 = _mm_mullo_epi16(a0, c1);
   c1 = _mm_srli_epi32(c1, 8);
   c1 = _mm_and_si128(GA_MASK_SSE3, c1);

   /* combine */
   return _mm_add_epi32(c0, c1);
}

static always_inline __m128i
sub4_alpha_sse3(__m128i c) {

   __m128i c0 = c;

   c0 = _mm_srli_epi32(c0, 24);
   return _mm_sub_epi32(ALPHA_SSE3, c0);
}

static always_inline __m128i
interp4_256_sse3(__m128i a, __m128i c0, __m128i c1)
{
   const __m128i zero = _mm_setzero_si128();

   __m128i a_l = a;
   __m128i a_h = a;
   a_l = _mm_unpacklo_epi16(a_l, a_l);
   a_h = _mm_unpackhi_epi16(a_h, a_h);

   __m128i a_t = _mm_slli_epi64(a_l, 32);
   __m128i a_t0 = _mm_slli_epi64(a_h, 32);

   a_l = _mm_add_epi32(a_l, a_t);
   a_h = _mm_add_epi32(a_h, a_t0);

   __m128i c0_l = c0;
   __m128i c0_h = c0;

   c0_l = _mm_unpacklo_epi8(c0_l, zero);
   c0_h = _mm_unpackhi_epi8(c0_h, zero);

   __m128i c1_l = c1;
   __m128i c1_h = c1;

   c1_l = _mm_unpacklo_epi8(c1_l, zero);
   c1_h = _mm_unpackhi_epi8(c1_h, zero);

   __m128i cl_sub = _mm_sub_epi16(c0_l, c1_l);
   __m128i ch_sub = _mm_sub_epi16(c0_h, c1_h);

   cl_sub = _mm_mullo_epi16(cl_sub, a_l);
   ch_sub = _mm_mullo_epi16(ch_sub, a_h);

   __m128i c1ls = _mm_slli_epi16(c1_l, 8);
   __m128i c1hs = _mm_slli_epi16(c1_h, 8);

   cl_sub = _mm_add_epi16(cl_sub, c1ls);
   ch_sub = _mm_add_epi16(ch_sub, c1hs);

   cl_sub = _mm_and_si128(cl_sub, RB_MASK_SSE3);
   ch_sub = _mm_and_si128(ch_sub, RB_MASK_SSE3);

   cl_sub = _mm_srli_epi64(cl_sub, 8);
   ch_sub = _mm_srli_epi64(ch_sub, 8);

   cl_sub = _mm_packus_epi16(cl_sub, cl_sub);
   ch_sub = _mm_packus_epi16(ch_sub, ch_sub);

   return  (__m128i) _mm_shuffle_ps( (__m128)cl_sub, (__m128)ch_sub, 0x44);
}

static always_inline __m128i
mul_sym_sse3(__m128i a, __m128i c) {

      /* Prepare alpha for word mult */
      __m128i a_l = a;
      __m128i a_h = a;
      a_l = _mm_unpacklo_epi16(a_l, a_l);
      a_h = _mm_unpackhi_epi16(a_h, a_h);
      __m128i a0 = (__m128i) _mm_shuffle_ps( (__m128)a_l, (__m128)a_h, 0x88);

      /* first part */
      __m128i c0 = c;
      c0 = _mm_srli_epi32(c0, 8);
      c0 = _mm_and_si128(GA_MASK_SSE3, c0);
      c0 = _mm_mullo_epi16(a0, c0);
      c0 = _mm_add_epi32(c0, GA_MASK_SSE3);
      c0 = _mm_and_si128(RB_MASK_SSE3, c0);

      /* second part */
      __m128i c1 = c;
      c1 = _mm_and_si128(GA_MASK_SSE3, c1);
      c1 = _mm_mullo_epi16(a0, c1);
      c1 = _mm_add_epi32(c1, GA_MASK_SSE3);
      c1 = _mm_srli_epi32(c1, 8);
      c1 = _mm_and_si128(GA_MASK_SSE3, c1);

      return _mm_add_epi32(c0, c1);
}

static always_inline __m128i
mul4_sym_sse3(__m128i x, __m128i y) {

   const __m128i zero = _mm_setzero_si128();

   __m128i x_l = _mm_unpacklo_epi8(x, zero);
   __m128i x_h = _mm_unpackhi_epi8(x, zero);

   __m128i y_l = _mm_unpacklo_epi8(y, zero);
   __m128i y_h = _mm_unpackhi_epi8(y, zero);

   __m128i r_l = _mm_mullo_epi16(x_l, y_l);
   __m128i r_h = _mm_mullo_epi16(x_h, y_h);

   r_l = _mm_add_epi16(r_l, SYM4_MASK_SSE3);
   r_h = _mm_add_epi16(r_h, SYM4_MASK_SSE3);

   r_l = _mm_srli_epi16(r_l, 8);
   r_h = _mm_srli_epi16(r_h, 8);

   return  _mm_packus_epi16(r_l, r_h);
}

static always_inline __m128i
mul3_sym_sse3(__m128i x, __m128i y) {

   __m128i res = mul4_sym_sse3(x, y);
   return  _mm_and_si128(res, RGB_MASK_SSE3);
}

#define LOOP_ALIGNED_U1_A48_SSE3(D, LENGTH, UOP,A4OP, A8OP) \
   { \
      while((uintptr_t)d & 0xF && l) UOP \
   \
      while(l) { \
         switch(l) { \
            case 3: UOP \
            case 2: UOP \
            case 1: UOP \
               break; \
            case 7: \
            case 6: \
            case 5: \
            case 4: \
               A4OP \
               break; \
            default: \
               A8OP \
               break; \
         } \
      } \
   }


#endif

#endif
