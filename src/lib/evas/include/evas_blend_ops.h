#ifndef EVAS_BLEND_OPS_H
#define EVAS_BLEND_OPS_H

#include "config.h"

#if defined BUILD_MMX || defined BUILD_SSE
#include "evas_mmx.h"
#endif

#ifdef NEED_SSE3
# if defined BUILD_SSE3
#  include <immintrin.h>
# endif
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
/* CPU AVX2 */
#define CPU_AVX2 7
/* cpu flags count */
#define CPU_LAST 8


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
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

#define MUL3_SYM(x, y) \
 ( ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
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

/* some useful NEON macros */

#ifdef BUILD_NEON
#define FPU_NEON \
	__asm__ __volatile__(".fpu neon \n\t");

/* copy reg1 to reg2 */
#define VMOV_R2R_NEON(reg1, reg2) \
	__asm__ __volatile__("vmov " #reg1 ", " #reg2 " \n\t" ::: #reg1);

/* copy 32bit value to lower bits of register reg */
#define VMOV_M2R_NEON(reg, value) \
	__asm__ __volatile__("vmov.32 " #reg "[0], %[val] \n\t" :: [val] "r" (value) : #reg);

/* save 32bit value from lower 64 bits of register regq to memory location */
/* pointed to by pointer, using 64bit register regd as temporary location */
#define VMOV_R2M_NEON(regq, regd, pointer) \
	__asm__ __volatile__("vqmovn.u16 " #regd ", " #regq " \n\t" \
			     "vst1.32 {" #regd "[0]}, [%[p]] \n\t" :: [p] "r" (pointer) : #regd, "memory");

/* spread constant imm in register reg */
#define VMOV_I2R_NEON(reg, imm) \
	__asm__ __volatile__("vmov.i16 " #reg ", " #imm " \n\t" ::: #reg);

/* spread value in register reg */
#define VDUP_NEON(reg, value) \
	__asm__ __volatile__("vdup.16 " #reg ", %[val] \n\t" :: [val] "r" (value) : #reg);

/* interleave contents of reg1 and reg2 */
#define VZIP_NEON(reg1, reg2) \
	__asm__ __volatile__("vzip.8 " #reg1 ", " #reg2 " \n\t" ::: #reg1 , #reg2);

/* swap contents of two registers */
#define VSWP_NEON(reg1, reg2) \
	__asm__ __volatile__("vswp " #reg1 ", " #reg2 " \n\t" ::: #reg1 , #reg2);

/* set register to zero */
#define VEOR_NEON(reg) \
	__asm__ __volatile__("veor " #reg ", " #reg ", " #reg " \n\t" ::: #reg);

/* do interpolation of every channel RGBA, result is contained in regy */
#define INTERP_256_NEON(rega, regx, regy, reg255) \
	__asm__ __volatile__("vsub.i16 " #regx ", " #regx ", " #regy " \n\t" \
			     "vmul.u16 " #regx ", " #regx ", " #rega " \n\t" \
			     "vsri.16 " #regx ", " #regx ", #8 \n\t" \
			     "vadd.i16 " #regx ", " #regx ", " #regy " \n\t" \
			     "vand " #regy ", " #regx ", " #reg255 " \n\t" \
			     ::: #regx, #regy );

/* multiply every channel of regx and regy */
#define MUL4_SYM_NEON(regx, regy, reg255) \
	__asm__ __volatile__("vmul.u16 " #regx ", " #regx ", " #regy " \n\t" \
			     "vadd.i16 " #regx ", " #regx ", " #reg255 " \n\t" \
			     "vsri.16 " #regx ", " #regx ", #8 \n\t" \
			     "vand " #regx ", " #regx ", " #reg255 " \n\t" \
			     ::: #regx );

#endif

/* some useful SSE3 inline functions */

#ifdef NEED_SSE3
#ifdef BUILD_SSE3

static __m128i GA_MASK_SSE3;
static __m128i RB_MASK_SSE3;
static __m128i SYM4_MASK_SSE3;
static __m128i RGB_MASK_SSE3;
//static __m128i A_MASK_SSE3;

static __m128i ALPHA_SSE3;

#ifndef EFL_ALWAYS_INLINE
# define EFL_ALWAYS_INLINE inline
#endif

static EFL_ALWAYS_INLINE __m128i
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

static EFL_ALWAYS_INLINE __m128i
sub4_alpha_sse3(__m128i c) {

   __m128i c0 = c;

   c0 = _mm_srli_epi32(c0, 24);
   return _mm_sub_epi32(ALPHA_SSE3, c0);
}

static EFL_ALWAYS_INLINE __m128i
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

static EFL_ALWAYS_INLINE __m128i
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

static EFL_ALWAYS_INLINE __m128i
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

static EFL_ALWAYS_INLINE __m128i
mul3_sym_sse3(__m128i x, __m128i y) {

   __m128i res = mul4_sym_sse3(x, y);
   return  _mm_and_si128(res, RGB_MASK_SSE3);
}

#endif
#endif

/* some useful AVX2 inline functions */

#ifdef NEED_AVX2
#ifdef BUILD_AVX2

#include <immintrin.h>

static __m256i GA_MASK_AVX2;
static __m256i RB_MASK_AVX2;
static __m256i ALPHA_AVX2;
static __m256i SYM4_MASK_AVX2;
static __m256i RGB_MASK_AVX2;

#ifndef EFL_ALWAYS_INLINE
# define EFL_ALWAYS_INLINE inline
#endif

/* Operation-for-operation port of mul_256_sse3. Every step below is lane-local
 * - unpacklo/unpackhi and shuffle_ps all work within each 128-bit half on AVX2
 * - so this computes exactly what the SSE3 helper computes, twice, and the
 * results are bit-identical rather than merely equivalent. */
static EFL_ALWAYS_INLINE __m256i
mul_256_avx2(__m256i a, __m256i c) {

   /* prepare alpha for word multiplication */
   __m256i a_l = a;
   __m256i a_h = a;
   a_l = _mm256_unpacklo_epi16(a_l, a_l);
   a_h = _mm256_unpackhi_epi16(a_h, a_h);
   __m256i a0 = (__m256i) _mm256_shuffle_ps( (__m256)a_l, (__m256)a_h, 0x88);

   /* first half of calc */
   __m256i c0 = c;
   c0 = _mm256_srli_epi32(c0, 8);
   c0 = _mm256_and_si256(GA_MASK_AVX2, c0);
   c0 = _mm256_mullo_epi16(a0, c0);
   c0 = _mm256_and_si256(RB_MASK_AVX2, c0);

   /* second half of calc */
   __m256i c1 = c;
   c1 = _mm256_and_si256(GA_MASK_AVX2, c1);
   c1 = _mm256_mullo_epi16(a0, c1);
   c1 = _mm256_srli_epi32(c1, 8);
   c1 = _mm256_and_si256(GA_MASK_AVX2, c1);

   /* combine */
   return _mm256_add_epi32(c0, c1);
}

static EFL_ALWAYS_INLINE __m256i
sub4_alpha_avx2(__m256i c) {

   __m256i c0 = c;

   c0 = _mm256_srli_epi32(c0, 24);
   return _mm256_sub_epi32(ALPHA_AVX2, c0);
}

/* Operation-for-operation port of mul_sym_sse3. Same lane-local reasoning as
 * mul_256_avx2: unpacklo/unpackhi_epi16 and shuffle_ps(0x88) stay within each
 * 128-bit half, so this is the SSE3 helper computed twice in parallel. */
static EFL_ALWAYS_INLINE __m256i
mul_sym_avx2(__m256i a, __m256i c) {

      /* Prepare alpha for word mult */
      __m256i a_l = a;
      __m256i a_h = a;
      a_l = _mm256_unpacklo_epi16(a_l, a_l);
      a_h = _mm256_unpackhi_epi16(a_h, a_h);
      __m256i a0 = (__m256i) _mm256_shuffle_ps( (__m256)a_l, (__m256)a_h, 0x88);

      /* first part */
      __m256i c0 = c;
      c0 = _mm256_srli_epi32(c0, 8);
      c0 = _mm256_and_si256(GA_MASK_AVX2, c0);
      c0 = _mm256_mullo_epi16(a0, c0);
      c0 = _mm256_add_epi32(c0, GA_MASK_AVX2);
      c0 = _mm256_and_si256(RB_MASK_AVX2, c0);

      /* second part */
      __m256i c1 = c;
      c1 = _mm256_and_si256(GA_MASK_AVX2, c1);
      c1 = _mm256_mullo_epi16(a0, c1);
      c1 = _mm256_add_epi32(c1, GA_MASK_AVX2);
      c1 = _mm256_srli_epi32(c1, 8);
      c1 = _mm256_and_si256(GA_MASK_AVX2, c1);

      return _mm256_add_epi32(c0, c1);
}

/* Operation-for-operation port of mul4_sym_sse3. Lane-crossing check: on
 * AVX2, _mm256_unpacklo_epi8(x, zero) interleaves bytes 0-7 of EACH 128-bit
 * lane with zero (never bytes 16-23 mixing with 0-7), and unpackhi does the
 * same for bytes 8-15 of each lane. A YMM register holds 8 DATA32 pixels, 4
 * per 128-bit lane, so unpacklo/unpackhi split each lane's 4 pixels into its
 * first 2 (bytes 0-7) and last 2 (bytes 8-15) - never crossing into the other
 * lane's pixels. _mm256_packus_epi16(r_l, r_h) reassembles each lane's words
 * back into bytes in the same low/high order per lane. So the round trip
 * preserves per-lane element order and lane0 (pixels 0-3) never mixes with
 * lane1 (pixels 4-7): the port is a straightforward doubling, verified
 * against the differential test below. */
static EFL_ALWAYS_INLINE __m256i
mul4_sym_avx2(__m256i x, __m256i y) {

   const __m256i zero = _mm256_setzero_si256();

   __m256i x_l = _mm256_unpacklo_epi8(x, zero);
   __m256i x_h = _mm256_unpackhi_epi8(x, zero);

   __m256i y_l = _mm256_unpacklo_epi8(y, zero);
   __m256i y_h = _mm256_unpackhi_epi8(y, zero);

   __m256i r_l = _mm256_mullo_epi16(x_l, y_l);
   __m256i r_h = _mm256_mullo_epi16(x_h, y_h);

   r_l = _mm256_add_epi16(r_l, SYM4_MASK_AVX2);
   r_h = _mm256_add_epi16(r_h, SYM4_MASK_AVX2);

   r_l = _mm256_srli_epi16(r_l, 8);
   r_h = _mm256_srli_epi16(r_h, 8);

   return  _mm256_packus_epi16(r_l, r_h);
}

static EFL_ALWAYS_INLINE __m256i
mul3_sym_avx2(__m256i x, __m256i y) {

   __m256i res = mul4_sym_avx2(x, y);
   return  _mm256_and_si256(res, RGB_MASK_AVX2);
}

/* Operation-for-operation port of interp4_256_sse3. unpacklo/unpackhi_epi8,
 * slli_epi64/srli_epi64 (64-bit lanes, never crossing the 128-bit halves),
 * and shuffle_ps(0x44) are all lane-local on AVX2, by the same reasoning as
 * mul_256_avx2 and mul4_sym_avx2 above. */
static EFL_ALWAYS_INLINE __m256i
interp4_256_avx2(__m256i a, __m256i c0, __m256i c1)
{
   const __m256i zero = _mm256_setzero_si256();

   __m256i a_l = a;
   __m256i a_h = a;
   a_l = _mm256_unpacklo_epi16(a_l, a_l);
   a_h = _mm256_unpackhi_epi16(a_h, a_h);

   __m256i a_t = _mm256_slli_epi64(a_l, 32);
   __m256i a_t0 = _mm256_slli_epi64(a_h, 32);

   a_l = _mm256_add_epi32(a_l, a_t);
   a_h = _mm256_add_epi32(a_h, a_t0);

   __m256i c0_l = c0;
   __m256i c0_h = c0;

   c0_l = _mm256_unpacklo_epi8(c0_l, zero);
   c0_h = _mm256_unpackhi_epi8(c0_h, zero);

   __m256i c1_l = c1;
   __m256i c1_h = c1;

   c1_l = _mm256_unpacklo_epi8(c1_l, zero);
   c1_h = _mm256_unpackhi_epi8(c1_h, zero);

   __m256i cl_sub = _mm256_sub_epi16(c0_l, c1_l);
   __m256i ch_sub = _mm256_sub_epi16(c0_h, c1_h);

   cl_sub = _mm256_mullo_epi16(cl_sub, a_l);
   ch_sub = _mm256_mullo_epi16(ch_sub, a_h);

   __m256i c1ls = _mm256_slli_epi16(c1_l, 8);
   __m256i c1hs = _mm256_slli_epi16(c1_h, 8);

   cl_sub = _mm256_add_epi16(cl_sub, c1ls);
   ch_sub = _mm256_add_epi16(ch_sub, c1hs);

   cl_sub = _mm256_and_si256(cl_sub, RB_MASK_AVX2);
   ch_sub = _mm256_and_si256(ch_sub, RB_MASK_AVX2);

   cl_sub = _mm256_srli_epi64(cl_sub, 8);
   ch_sub = _mm256_srli_epi64(ch_sub, 8);

   cl_sub = _mm256_packus_epi16(cl_sub, cl_sub);
   ch_sub = _mm256_packus_epi16(ch_sub, ch_sub);

   return  (__m256i) _mm256_shuffle_ps( (__m256)cl_sub, (__m256)ch_sub, 0x44);
}

#endif
#endif

#define LOOP_ALIGNED_U1_A48(DEST, LENGTH, UOP, A4OP, A8OP) \
  {                                                        \
      while((uintptr_t)DEST & 0xF && LENGTH) UOP \
   \
      while(LENGTH) { \
        switch(LENGTH) {                        \
          case 3: UOP; EINA_FALLTHROUGH;        \
          case 2: UOP; EINA_FALLTHROUGH;        \
          case 1: UOP;                          \
           break;                               \
          case 7:                               \
           EINA_FALLTHROUGH;                    \
          case 6:                               \
           EINA_FALLTHROUGH;                    \
          case 5:                               \
           EINA_FALLTHROUGH;                    \
          case 4:                               \
           A4OP                                 \
           break;                               \
          default:                              \
           A8OP                                 \
           break;                               \
        }                                       \
      } \
   }

/* Same shape as LOOP_ALIGNED_U1_A48 but for 256-bit kernels: scalar until DEST
 * reaches a 16-byte boundary, then (if needed) one 4-wide __m128i block to
 * reach a 32-byte boundary, then 16 pixels at a time, then 8, then a 4-wide
 * block again for a 4-7 pixel remainder, then scalar for whatever is left
 * (1-3 pixels).
 *
 * The 4-wide stage exists so AVX2 and SSE3 (LOOP_ALIGNED_U1_A48) classify
 * every pixel the same way - scalar or vector - which matters because some
 * AVX2 helpers (mul4_sym_avx2 etc.) are wide ports of an SSE3 kernel that is
 * a known rounding approximation relative to plain C: any pixel processed by
 * *any* vector width (4/8/16) gets that same approximation bit-for-bit
 * (mul4_sym_avx2 is bit-identical to mul4_sym_sse3, wide-for-wide), while a
 * pixel processed by the scalar UOP is exact against C instead. So the two
 * tiers are only guaranteed to agree if they draw the scalar/vector line at
 * the same pixel.
 *
 * SSE3 draws that line based purely on alignment-to-16-bytes and remaining
 * count: prologue to 16-byte alignment, then vector-process every pixel
 * down to a less-than-4 remainder. Naively gating AVX2's *entire* prologue
 * on 32-byte alignment (as an earlier version of this macro did) does not
 * reproduce that: for a short, 16-but-not-32-byte-aligned span (e.g. a
 * 4-pixel span at a 16-aligned, non-32-aligned address - an ordinary
 * offset the differential test's length/offset sweep hits, and real render
 * spans hit too), the 32-byte prologue would consume the *entire* span as
 * scalar before ever reaching a vector op, while SSE3, needing only
 * 16-byte alignment, takes its approximate 4-wide path for the same
 * pixels - a disagreement.
 *
 * This macro instead: (1) aligns to 16 bytes exactly like SSE3's own
 * prologue, so the two tiers agree pixel-for-pixel on how many leading
 * pixels are scalar; (2) if still short of 32-byte alignment and at least 4
 * pixels remain, runs exactly one 4-wide block to close the last 16 bytes -
 * using the *same* arithmetic (SSE3's A4OP, when a kernel's A4OP is written
 * that way) SSE3 would apply to those same 4 pixels regardless, so this
 * costs nothing in exactness and satisfies _mm256_load/store_si256's 32-byte
 * requirement for what follows; (3) proceeds with 16/8-wide blocks and a
 * final 4-7 remainder exactly as before. Every pixel past the 16-byte-align
 * prologue that SSE3 would vector-process, AVX2 also vector-processes
 * (in some width - which width doesn't matter, since all of them agree
 * bit-for-bit), and every pixel SSE3 leaves as a <4 scalar tail, AVX2 does
 * too. */
#define LOOP_ALIGNED_U1_A4_A8_A16(DEST, LENGTH, UOP, A4OP, A8OP, A16OP) \
  {                                                            \
      while((uintptr_t)DEST & 0xF && LENGTH) UOP \
   \
      if(((uintptr_t)DEST & 0x1F) && LENGTH >= 4) A4OP \
   \
      while(LENGTH) { \
        switch(LENGTH) {                        \
          case 3: UOP; EINA_FALLTHROUGH;        \
          case 2: UOP; EINA_FALLTHROUGH;        \
          case 1: UOP;                          \
           break;                               \
          case 7:                               \
           EINA_FALLTHROUGH;                    \
          case 6:                               \
           EINA_FALLTHROUGH;                    \
          case 5:                               \
           EINA_FALLTHROUGH;                    \
          case 4:                               \
           A4OP                                 \
           break;                               \
          case 15:                              \
           EINA_FALLTHROUGH;                    \
          case 14:                              \
           EINA_FALLTHROUGH;                    \
          case 13:                              \
           EINA_FALLTHROUGH;                    \
          case 12:                              \
           EINA_FALLTHROUGH;                    \
          case 11:                              \
           EINA_FALLTHROUGH;                    \
          case 10:                              \
           EINA_FALLTHROUGH;                    \
          case 9:                               \
           EINA_FALLTHROUGH;                    \
          case 8:                               \
           A8OP                                 \
           break;                               \
          default:                              \
           A16OP                                \
           break;                               \
        }                                       \
      } \
   }

#endif
