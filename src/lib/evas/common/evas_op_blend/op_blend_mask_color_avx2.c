/* blend mask x color -> dst */

/* Port of op_blend_mask_color_sse3.c's two span kernels
 * (_op_blend_mas_c_dp_sse3, _op_blend_mas_can_dp_sse3). mul_sym_sse3/
 * mul_sym_avx2 and interp4_256_sse3/interp4_256_avx2 agree with their C
 * counterparts exactly for every SM_AS slot, so every slot registered below
 * must be bit-exact against BOTH C and SSE3 - there is no rounding-gap
 * excuse available here the way there is for the SC/SC_AN pixel+colour
 * kernels in op_blend_pixel_color_avx2.c.
 *
 * As in that file, the A4OP stage runs the SSE3 kernel's 4-wide body
 * verbatim (mul_sym_sse3/interp4_256_sse3, __m128i) so a 4-7 pixel span
 * takes the identical instructions on both tiers; A8OP/A16OP use the AVX2
 * counterparts mul_sym_avx2/interp4_256_avx2 from evas_blend_ops.h.
 *
 * MASK WIDENING: m is DATA8* (1 byte/pixel). A8OP widens 8 mask bytes with
 * _mm256_cvtepu8_epi32(_mm_loadl_epi64(...)); A16OP does the same for m and
 * m+8 - byte i always lands in 32-bit lane i, no lane-crossing shuffle
 * involved. */

#ifdef BUILD_AVX2

static void
_op_blend_mas_c_dp_avx2(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP - byte-identical to SSE3's UOP */

         DATA32 a = *m;
         DATA32 mc = MUL_SYM(a, c);
         a = 256 - (mc >> 24);
         *d = mc + MUL_256(a, *d);
         m++;  d++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body (continue -> if/else: the
           pre-alignment stage of LOOP_ALIGNED_U1_A4_A8_A16 invokes A4OP
           outside any loop, where `continue` is illegal) */

         if ((m[3] | m[2] | m[1] | m[0]) != 0) {
            __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);
            __m128i d0 = _mm_load_si128((__m128i *)d);

            __m128i mc0 = mul_sym_sse3(m0, c_packed128);
            __m128i  a0 = sub4_alpha_sse3(mc0);
            __m128i mul0 = mul_256_sse3(a0, d0);

            mul0 = _mm_add_epi32(mul0, mc0);

            _mm_store_si128((__m128i *)d, mul0);
         }

         m += 4; d += 4; l -= 4;
      },
      { /* A8OP */

         if ((m[7] | m[6] | m[5] | m[4] | m[3] | m[2] | m[1] | m[0]) != 0) {
            __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));
            __m256i d0 = _mm256_load_si256((__m256i *)d);

            __m256i mc0 = mul_sym_avx2(m0, c_packed);
            __m256i a0  = sub4_alpha_avx2(mc0);
            __m256i mul0 = mul_256_avx2(a0, d0);

            mul0 = _mm256_add_epi32(mul0, mc0);

            _mm256_store_si256((__m256i *)d, mul0);
         }

         m += 8; d += 8; l -= 8;
      },
      { /* A16OP */

         if ((m[7] | m[6] | m[5] | m[4] | m[3] | m[2] | m[1] | m[0] |
              m[15] | m[14] | m[13] | m[12] | m[11] | m[10] | m[9] | m[8]) != 0) {
            __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));
            __m256i d0 = _mm256_load_si256((__m256i *)d);

            __m256i m1 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)(m+8)));
            __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

            __m256i mc0 = mul_sym_avx2(m0, c_packed);
            __m256i a0  = sub4_alpha_avx2(mc0);
            __m256i mul0 = mul_256_avx2(a0, d0);

            mul0 = _mm256_add_epi32(mc0, mul0);

            __m256i mc1 = mul_sym_avx2(m1, c_packed);
            __m256i a1  = sub4_alpha_avx2(mc1);
            __m256i mul1 = mul_256_avx2(a1, d1);

            mul1 = _mm256_add_epi32(mc1, mul1);

            _mm256_store_si256((__m256i *)d, mul0);
            _mm256_store_si256((__m256i *)(d+8), mul1);
         }

         m += 16; d += 16; l -= 16;
      })
}

static void
_op_blend_mas_can_dp_avx2(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {

   DATA32 alpha;

   const __m256i one = _mm256_set1_epi32(1);
   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i one128 = _mm_set_epi32(1, 1, 1, 1);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP - byte-identical to SSE3's UOP */

         alpha = *m;
         switch(alpha)
           {
           case 0:
              break;
           case 255:
              *d = c;
              break;
           default:
              alpha++;
              *d = INTERP_256(alpha, c, *d);
              break;
           }
         m++;  d++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body (continue -> if/else, see above) */

         if ((m[3] | m[2] | m[1] | m[0]) != 0) {
            __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);
            __m128i d0 = _mm_load_si128((__m128i *)d);

            __m128i zm0 = _mm_cmpeq_epi32(m0, _mm_setzero_si128());

            m0 = _mm_add_epi32(one128, m0);

            __m128i r0 = interp4_256_sse3(m0, c_packed128, d0);

            r0 = _mm_and_si128(~zm0, r0);
            d0 = _mm_and_si128(zm0, d0);

            d0 = _mm_add_epi32(r0, d0);

            _mm_store_si128((__m128i *)d, d0);
         }

         m += 4; d += 4; l -= 4;
      },
      { /* A8OP */

         if ((m[7] | m[6] | m[5] | m[4] | m[3] | m[2] | m[1] | m[0]) != 0) {
            __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));
            __m256i d0 = _mm256_load_si256((__m256i *)d);

            __m256i zm0 = _mm256_cmpeq_epi32(m0, _mm256_setzero_si256());

            m0 = _mm256_add_epi32(one, m0);

            __m256i r0 = interp4_256_avx2(m0, c_packed, d0);

            r0 = _mm256_andnot_si256(zm0, r0);
            d0 = _mm256_and_si256(zm0, d0);

            d0 = _mm256_add_epi32(r0, d0);

            _mm256_store_si256((__m256i *)d, d0);
         }

         m += 8; d += 8; l -= 8;
      },
      { /* A16OP */

         if ((m[7] | m[6] | m[5] | m[4] | m[3] | m[2] | m[1] | m[0] |
              m[15] | m[14] | m[13] | m[12] | m[11] | m[10] | m[9] | m[8]) != 0) {
            __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));
            __m256i d0 = _mm256_load_si256((__m256i *)d);

            __m256i m1 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)(m+8)));
            __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

            __m256i zm0 = _mm256_cmpeq_epi32(m0, _mm256_setzero_si256());
            __m256i zm1 = _mm256_cmpeq_epi32(m1, _mm256_setzero_si256());

            m0 = _mm256_add_epi32(one, m0);
            m1 = _mm256_add_epi32(one, m1);

            __m256i r0 = interp4_256_avx2(m0, c_packed, d0);
            __m256i r1 = interp4_256_avx2(m1, c_packed, d1);

            r0 = _mm256_andnot_si256(zm0, r0);
            d0 = _mm256_and_si256(zm0, d0);

            r1 = _mm256_andnot_si256(zm1, r1);
            d1 = _mm256_and_si256(zm1, d1);

            d0 = _mm256_add_epi32(d0, r0);
            d1 = _mm256_add_epi32(d1, r1);

            _mm256_store_si256((__m256i *)d, d0);
            _mm256_store_si256((__m256i *)(d+8), d1);
         }

         m += 16; d += 16; l -= 16;
      })
}

#define _op_blend_mas_cn_dp_avx2 _op_blend_mas_can_dp_avx2
#define _op_blend_mas_caa_dp_avx2 _op_blend_mas_c_dp_avx2

#define _op_blend_mas_c_dpan_avx2 _op_blend_mas_c_dp_avx2
#define _op_blend_mas_cn_dpan_avx2 _op_blend_mas_cn_dp_avx2
#define _op_blend_mas_can_dpan_avx2 _op_blend_mas_can_dp_avx2
#define _op_blend_mas_caa_dpan_avx2 _op_blend_mas_caa_dp_avx2

static void
init_blend_mask_color_span_funcs_avx2(void)
{
   /* SSE3 leaves these slots disabled (FIXMEs naming pre-Haswell CPUs that
    * cannot reach an AVX2 path); AVX2 registers them - bit-exact vs C. */
   op_blend_span_funcs[SP_N][SM_AS][SC][DP][CPU_AVX2] = _op_blend_mas_c_dp_avx2;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_AVX2] = _op_blend_mas_cn_dp_avx2;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_AVX2] = _op_blend_mas_can_dp_avx2;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_AVX2] = _op_blend_mas_caa_dp_avx2;

   op_blend_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_AVX2] = _op_blend_mas_c_dpan_avx2;
   op_blend_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_AVX2] = _op_blend_mas_cn_dpan_avx2;
   op_blend_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_AVX2] = _op_blend_mas_can_dpan_avx2;
   op_blend_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_AVX2] = _op_blend_mas_caa_dpan_avx2;
}

/*-----*/

/* blend_rel mask x color -> dst */

static void
_op_blend_rel_mas_c_dp_avx2(DATA32 *s EINA_UNUSED, DATA8 *m, DATA32 c, DATA32 *d, int l) {

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP - byte-identical to SSE3's UOP */

         DATA32 mc = MUL_SYM(*m, c);
         int alpha = 256 - (mc >> 24);
         *d = MUL_SYM(*d >> 24, mc) + MUL_256(alpha, *d);
         d++; m++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);
         __m128i d0 = _mm_load_si128((__m128i *) d);

         __m128i mc0 = mul_sym_sse3(m0, c_packed128);
         __m128i a0 = sub4_alpha_sse3(mc0);

         __m128i d0_sym = mul_sym_sse3(_mm_srli_epi32(d0, 24), mc0);
         d0 = mul_256_sse3(a0, d0);

         d0 = _mm_add_epi32(d0, d0_sym);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; m += 4; l -= 4;
      },
      { /* A8OP */

         __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i mc0 = mul_sym_avx2(m0, c_packed);
         __m256i a0 = sub4_alpha_avx2(mc0);

         __m256i d0_sym = mul_sym_avx2(_mm256_srli_epi32(d0, 24), mc0);
         d0 = mul_256_avx2(a0, d0);

         d0 = _mm256_add_epi32(d0, d0_sym);

         _mm256_store_si256((__m256i *)d, d0);

         d += 8; m += 8; l -= 8;
      },
      { /* A16OP */

         __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i m1 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)(m+8)));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i mc0 = mul_sym_avx2(m0, c_packed);
         __m256i mc1 = mul_sym_avx2(m1, c_packed);

         __m256i a0 = sub4_alpha_avx2(mc0);
         __m256i a1 = sub4_alpha_avx2(mc1);

         __m256i d0_sym = mul_sym_avx2(_mm256_srli_epi32(d0, 24), mc0);
         __m256i d1_sym = mul_sym_avx2(_mm256_srli_epi32(d1, 24), mc1);

         d0 = mul_256_avx2(a0, d0);
         d1 = mul_256_avx2(a1, d1);

         d0 = _mm256_add_epi32(d0, d0_sym);
         d1 = _mm256_add_epi32(d1, d1_sym);

         _mm256_store_si256((__m256i *)d, d0);
         _mm256_store_si256((__m256i *)(d+8), d1);

         d += 16; m += 16; l -= 16;
      })
}

#define _op_blend_rel_mas_cn_dp_avx2 _op_blend_rel_mas_c_dp_avx2
#define _op_blend_rel_mas_can_dp_avx2 _op_blend_rel_mas_c_dp_avx2
#define _op_blend_rel_mas_caa_dp_avx2 _op_blend_rel_mas_c_dp_avx2

#define _op_blend_rel_mas_c_dpan_avx2 _op_blend_mas_c_dpan_avx2
#define _op_blend_rel_mas_cn_dpan_avx2 _op_blend_mas_cn_dpan_avx2
#define _op_blend_rel_mas_can_dpan_avx2 _op_blend_mas_can_dpan_avx2
#define _op_blend_rel_mas_caa_dpan_avx2 _op_blend_mas_caa_dpan_avx2

static void
init_blend_rel_mask_color_span_funcs_avx2(void)
{
   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP][CPU_AVX2] = _op_blend_rel_mas_c_dp_avx2;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP][CPU_AVX2] = _op_blend_rel_mas_can_dp_avx2;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP][CPU_AVX2] = _op_blend_rel_mas_can_dp_avx2;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP][CPU_AVX2] = _op_blend_rel_mas_caa_dp_avx2;

   op_blend_rel_span_funcs[SP_N][SM_AS][SC][DP_AN][CPU_AVX2] = _op_blend_rel_mas_c_dpan_avx2;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_N][DP_AN][CPU_AVX2] = _op_blend_rel_mas_cn_dpan_avx2;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AN][DP_AN][CPU_AVX2] = _op_blend_rel_mas_can_dpan_avx2;
   op_blend_rel_span_funcs[SP_N][SM_AS][SC_AA][DP_AN][CPU_AVX2] = _op_blend_rel_mas_caa_dpan_avx2;
}

#endif
