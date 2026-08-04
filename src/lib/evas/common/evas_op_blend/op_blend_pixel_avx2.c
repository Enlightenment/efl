/* blend pixel --> dst */

/* What the differential test does and does not prove for this file: the four
 * slots registered below are backed by a single kernel function (plus its
 * SP_AS alias), at one arithmetic shape - plain premultiplied source blended
 * over dest, no mask, no colour. A bit-exact pass here validates the porting
 * pattern (mul_256_avx2/sub4_alpha_avx2 as lane-local ports of the SSE3
 * helpers) and the LOOP_ALIGNED_U1_A8_A16 alignment handling. It does NOT
 * validate any other arithmetic shape - mask blends, colour blends, the
 * relative-blend variants, etc. Later kernel groups that copy this file's
 * pattern each need their own differential-test run against their own C
 * reference; none of that verification can be inherited from this result.
 */

#ifdef BUILD_AVX2

static void
_op_blend_p_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d, int l) {

   LOOP_ALIGNED_U1_A8_A16(d, l,
      { /* UOP */

         int alpha = 256 - (*s >> 24);
         *d = *s + MUL_256(alpha, *d);
         s++; d++; l--;
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

#endif
