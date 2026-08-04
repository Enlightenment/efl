/* blend pixel --> dst */

/* What the differential test does and does not prove for this file: the four
 * slots registered below are backed by a single kernel function (plus its
 * SP_AS alias), at one arithmetic shape - plain premultiplied source blended
 * over dest, no mask, no colour. A bit-exact pass here validates the porting
 * pattern (mul_256_avx2/sub4_alpha_avx2 as lane-local ports of the SSE3
 * helpers) and the LOOP_ALIGNED_U1_A4_A8_A16 alignment handling. It does NOT
 * validate any other arithmetic shape - mask blends, colour blends, the
 * relative-blend variants, etc. Later kernel groups that copy this file's
 * pattern each need their own differential-test run against their own C
 * reference; none of that verification can be inherited from this result.
 *
 * The A4OP block below is the SSE3 kernel's A4OP body verbatim (__m128i,
 * _sse3 helpers) rather than a 128-bit-narrowed AVX2 port: the point of the
 * 4-wide stage is for AVX2 to execute literally the same instructions SSE3
 * does at 4-7 pixels, not merely equivalent ones. mul_256_sse3 is exact
 * against plain C at every width, so this doesn't change this kernel's
 * bit-exactness against C; it matters for kernels elsewhere in this group
 * whose SSE3 helpers are not exact against C (see op_blend_pixel_color_avx2.c).
 */

#ifdef BUILD_AVX2

static void
_op_blend_p_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d, int l) {

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         int alpha = 256 - (*s >> 24);
         *d = *s + MUL_256(alpha, *d);
         s++; d++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i a0 = sub4_alpha_sse3(s0);
         __m128i mul0 = mul_256_sse3(a0, d0);
         d0 = _mm_add_epi32(mul0, s0);

         _mm_store_si128((__m128i *)d, d0);

         s += 4; d += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i a0 = sub4_alpha_avx2(s0);
         __m256i mul0 = mul_256_avx2(a0, d0);
         d0 = _mm256_add_epi32(mul0, s0);

         _mm256_store_si256((__m256i *)d, d0);

         s += 8; d += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i a0 = sub4_alpha_avx2(s0);
         __m256i a1 = sub4_alpha_avx2(s1);

         __m256i mul0 = mul_256_avx2(a0, d0);
         __m256i mul1 = mul_256_avx2(a1, d1);

         d0 = _mm256_add_epi32(mul0, s0);
         d1 = _mm256_add_epi32(mul1, s1);

         _mm256_store_si256((__m256i *)d, d0);
         _mm256_store_si256((__m256i *)(d+8), d1);

         s += 16; d += 16; l -= 16;
      })
}

#define _op_blend_pas_dp_avx2 _op_blend_p_dp_avx2

static void
init_blend_pixel_span_funcs_avx2(void)
{
   op_blend_span_funcs[SP][SM_N][SC_N][DP][CPU_AVX2] = _op_blend_p_dp_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_AVX2] = _op_blend_pas_dp_avx2;

   /* SSE3 leaves this slot disabled (2011 FIXME); AVX2 registers it - bit-exact
    * vs C, and no pre-Haswell CPU can reach an AVX2 path. */
   op_blend_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_blend_p_dp_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_blend_pas_dp_avx2;
}

/*-----*/

/* blend_rel pixel -> dst */

static void
_op_blend_rel_p_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   const __m128i ones128 = _mm_set_epi32(1, 1, 1, 1);
   const __m256i ones = _mm256_set1_epi32(1);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         int alpha = 256 - (*s >> 24);
         c = 1 + (*d >> 24);
         *d = MUL_256(c, *s) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /*A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i c0 = _mm_add_epi32(_mm_srli_epi32(d0, 24), ones128);
         __m128i a0 = sub4_alpha_sse3(s0);

         d0 = _mm_add_epi32(mul_256_sse3(c0, s0), mul_256_sse3(a0, d0));

         _mm_store_si128((__m128i *)d, d0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i c0 = _mm256_add_epi32(_mm256_srli_epi32(d0, 24), ones);
         __m256i a0 = sub4_alpha_avx2(s0);

         d0 = _mm256_add_epi32(mul_256_avx2(c0, s0), mul_256_avx2(a0, d0));

         _mm256_store_si256((__m256i *)d, d0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i c0 = _mm256_add_epi32(_mm256_srli_epi32(d0, 24), ones);
         __m256i c1 = _mm256_add_epi32(_mm256_srli_epi32(d1, 24), ones);

         __m256i a0 = sub4_alpha_avx2(s0);
         __m256i a1 = sub4_alpha_avx2(s1);

         d0 = _mm256_add_epi32(mul_256_avx2(c0, s0), mul_256_avx2(a0, d0));
         d1 = _mm256_add_epi32(mul_256_avx2(c1, s1), mul_256_avx2(a1, d1));

         _mm256_store_si256((__m256i *)d, d0);
         _mm256_store_si256((__m256i *)(d+8), d1);

         d += 16; s += 16; l -= 16;
      })
}

static void
_op_blend_rel_pan_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   const __m128i ones128 = _mm_set_epi32(1, 1, 1, 1);
   const __m256i ones = _mm256_set1_epi32(1);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         c = 1 + (*d >> 24);
         *d++ = MUL_256(c, *s);
         s++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i c0 = _mm_add_epi32(_mm_srli_epi32(d0, 24), ones128);
         d0 = mul_256_sse3(c0, s0);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i c0 = _mm256_add_epi32(_mm256_srli_epi32(d0, 24), ones);
         d0 = mul_256_avx2(c0, s0);

         _mm256_store_si256((__m256i *)d, d0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i c0 = _mm256_add_epi32(_mm256_srli_epi32(d0, 24), ones);
         __m256i c1 = _mm256_add_epi32(_mm256_srli_epi32(d1, 24), ones);

         d0 = mul_256_avx2(c0, s0);
         d1 = mul_256_avx2(c1, s1);

         _mm256_store_si256((__m256i *)d, d0);
         _mm256_store_si256((__m256i *)(d+8), d1);

         d += 16; s += 16; l -= 16;
      })
}

#define _op_blend_rel_pas_dp_avx2 _op_blend_rel_p_dp_avx2

/* SSE3's DP_AN rel aliases point at its (non-rel) DP_AN kernels, which are
 * themselves aliases of the DP kernels (_op_blend_p_dpan_sse3 ==
 * _op_blend_p_dp_sse3, etc. - see op_blend_pixel_sse3.c). AVX2 registers
 * those DP_AN slots directly from the DP kernels rather than defining
 * separate _dpan_avx2 symbols, so alias straight to the same DP kernels here. */
#define _op_blend_rel_p_dpan_avx2 _op_blend_p_dp_avx2
#define _op_blend_rel_pan_dpan_avx2 NULL
#define _op_blend_rel_pas_dpan_avx2 _op_blend_pas_dp_avx2

static void
init_blend_rel_pixel_span_funcs_avx2(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_AVX2] = _op_blend_rel_p_dp_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_AVX2] = _op_blend_rel_pas_dp_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_AVX2] = _op_blend_rel_pan_dp_avx2;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_blend_rel_p_dpan_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_blend_rel_pas_dpan_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_blend_rel_pan_dpan_avx2;
}

#endif
