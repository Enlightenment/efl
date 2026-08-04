/* blend color -> dst */

/* The A4OP block is the SSE3 kernel's A4OP body verbatim (__m128i, _sse3
 * helpers): the 4-wide stage exists so AVX2 executes the same instructions
 * SSE3 does at 4-7 pixels. mul_256_sse3 is exact against plain C at every
 * width, so this doesn't change this kernel's bit-exactness against C.
 */

#ifdef BUILD_AVX2

static void
_op_blend_c_dp_avx2(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   DATA32 a = 256 - (c >> 24);

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m256i a_packed = _mm256_set1_epi32(a);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);
   const __m128i a_packed128 = _mm_set_epi32(a, a, a, a);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         *d = c + MUL_256(a, *d);
         d++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i d0 = _mm_load_si128((__m128i *)d);

         d0 = mul_256_sse3(a_packed128, d0);
         d0 = _mm_add_epi32(d0, c_packed128);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; l -= 4;
      },
      { /* A8OP */

         __m256i d0 = _mm256_load_si256((__m256i *)d);

         d0 = mul_256_avx2(a_packed, d0);
         d0 = _mm256_add_epi32(d0, c_packed);

         _mm256_store_si256((__m256i *)d, d0);

         d += 8; l -= 8;
      },
      { /* A16OP */

         __m256i d0 = _mm256_load_si256((__m256i *)d);
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         d0 = mul_256_avx2(a_packed, d0);
         d1 = mul_256_avx2(a_packed, d1);

         d0 = _mm256_add_epi32(d0, c_packed);
         d1 = _mm256_add_epi32(d1, c_packed);

         _mm256_store_si256((__m256i *)d, d0);
         _mm256_store_si256((__m256i *)(d+8), d1);

         d += 16; l -= 16;
      })
}

#define _op_blend_caa_dp_avx2 _op_blend_c_dp_avx2

#define _op_blend_c_dpan_avx2 _op_blend_c_dp_avx2
#define _op_blend_caa_dpan_avx2 _op_blend_c_dpan_avx2

static void
init_blend_color_span_funcs_avx2(void)
{
   /* SSE3 leaves these slots disabled (2011 FIXME); AVX2 registers them -
    * bit-exact vs C, and no pre-Haswell CPU can reach an AVX2 path. */
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_AVX2] = _op_blend_c_dp_avx2;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_caa_dp_avx2;

   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_c_dpan_avx2;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_caa_dpan_avx2;
}

/*-----*/

/* blend_rel color -> dst */

static void
_op_blend_rel_c_dp_avx2(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   int alpha = 256 - (c >> 24);

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m256i alpha_packed = _mm256_set1_epi32(alpha);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);
   const __m128i alpha_packed128 = _mm_set_epi32(alpha, alpha, alpha, alpha);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      {  /* UOP */

         *d = MUL_SYM(*d >> 24, c) + MUL_256(alpha, *d);
         d++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i mul0 = mul_256_sse3(alpha_packed128, d0);
         __m128i sym0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), c_packed128);

         d0 = _mm_add_epi32(mul0, sym0);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; l -= 4;
      },
      { /* A8OP */

         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i mul0 = mul_256_avx2(alpha_packed, d0);
         __m256i sym0 = mul_sym_avx2(_mm256_srli_epi32(d0, 24), c_packed);

         d0 = _mm256_add_epi32(mul0, sym0);

         _mm256_store_si256((__m256i *)d, d0);

         d += 8; l -= 8;
      },
      { /* A16OP */

         __m256i d0 = _mm256_load_si256((__m256i *)d);
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i mul0 = mul_256_avx2(alpha_packed, d0);
         __m256i mul1 = mul_256_avx2(alpha_packed, d1);

         __m256i sym0 = mul_sym_avx2(_mm256_srli_epi32(d0, 24), c_packed);
         __m256i sym1 = mul_sym_avx2(_mm256_srli_epi32(d1, 24), c_packed);

         d0 = _mm256_add_epi32(mul0, sym0);
         d1 = _mm256_add_epi32(mul1, sym1);

         _mm256_store_si256((__m256i *)d, d0);
         _mm256_store_si256((__m256i *)(d+8), d1);

         d += 16; l -= 16;
      })
}

#define _op_blend_rel_caa_dp_avx2 _op_blend_rel_c_dp_avx2
#define _op_blend_rel_c_dpan_avx2 _op_blend_c_dpan_avx2
#define _op_blend_rel_caa_dpan_avx2 _op_blend_caa_dpan_avx2

static void
init_blend_rel_color_span_funcs_avx2(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_AVX2] = _op_blend_rel_c_dp_avx2;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_rel_caa_dp_avx2;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_rel_c_dpan_avx2;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_rel_caa_dpan_avx2;
}

#endif
