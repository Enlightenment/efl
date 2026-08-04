/* blend pixel x mask --> dst */

/* Ported from the plain-C reference (op_blend_pixel_mask_.c), NOT from
 * op_blend_pixel_mask_sse3.c. The SSE3 file's two span kernels are compiled
 * out (#if 0) behind "// FIXME: These functions most likely don't perform
 * the correct operation. Test them with masks and images." with every one
 * of the six op-table slots redefined to NULL afterwards - so there is no
 * working, registered SSE3 kernel for this group to match bit-for-bit, and
 * the disabled body is explicitly flagged as suspect by whoever wrote it.
 *
 * The C reference itself is instructive here: _op_blend_p_mas_dp special
 * cases mask alpha 0 and 255 for speed, and its *default* case is exactly
 * MUL_SYM/MUL_256 composition:
 *
 *   c = MUL_SYM(alpha, *s); a = 256 - (c >> 24); *d = c + MUL_256(a, *d);
 *
 * which is algebraically identical to the special cases too - not merely a
 * fast path with different results:
 *   - alpha == 0:   MUL_SYM(0, x) == 0 for every channel (the +0xff00ff
 *     rounding term never carries past the low byte for a zero multiplier),
 *     so c == 0, a == 256, and MUL_256(256, d) == d exactly (multiplying by
 *     256 and shifting right 8 is the identity for an 8-bit channel) -> same
 *     result as the C special case's "*d unchanged".
 *   - alpha == 255: MUL_SYM(255, x) == x exactly for every channel (256x -
 *     x + 255, shifted right 8, is x + floor((255-x)/256) == x since 255-x
 *     < 256) -> same result as the C special case's "*d = *s + MUL_256(256 -
 *     (*s>>24), *d)".
 * So the unbranched general formula below reproduces the C reference for
 * every mask byte value, not just the ones that fall through its default
 * case, which is what makes an unconditional vector kernel viable here.
 *
 * mul_sym_avx2 and mul_256_avx2 (used below) are bit-exact against the
 * plain-C MUL_SYM/MUL_256 macros (mul_256_avx2 via the SC_AA "_caa_" kernels
 * in op_blend_pixel_color_avx2.c; mul_sym_avx2 has the identical
 * unpack/shuffle(0x88) structure and is bit-exact by the same argument).
 * Unlike mul4_sym/mul3_sym, they carry no known rounding gap versus their C
 * macros, so this port targets - and the evas_avx2_ops differential test
 * confirms - bit-exact-against-C, which is the correct bar here since there
 * is no SSE3 slot to match instead.
 *
 * The C reference aliases _op_blend_pas_mas_dp and _op_blend_pan_mas_dp
 * straight to _op_blend_p_mas_dp - source alpha state (SP/SP_AS/SP_AN)
 * does not change the formula, only the mask does. This file follows the
 * same aliasing, and likewise aliases the DP_AN destination variants
 * straight to the DP ones, exactly mirroring the C file's alias chain.
 *
 * MASK WIDENING: m is DATA8* (1 byte/pixel), s/d are DATA32* (4 bytes/
 * pixel). The A4OP stage below is the disabled SSE3 body's mask load
 * verbatim (_mm_set_epi32(m[3],m[2],m[1],m[0])), kept per the file-header
 * convention used throughout this port (Tasks 8/9) of running the SSE3
 * A4OP body byte-identical at 4-7 pixels regardless of source. For 8 and 16
 * pixels this file uses _mm256_cvtepu8_epi32 on a 64-bit/128-bit *load* of
 * mask bytes, not a shuffle: it is a pure widening convert (lane i of the
 * result is unpack(byte i of the source), for i in 0..7), so there is no
 * lane-crossing shuffle pattern to get wrong - byte 0 always lands in
 * 32-bit lane 0, byte 7 in lane 7, etc, confirmed by the differential test. */

#ifdef BUILD_AVX2

static void
_op_blend_p_mas_dp_avx2(DATA32 *s, DATA8 *m, DATA32 c EINA_UNUSED, DATA32 *d, int l) {

   int alpha;

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP - byte-identical to the C reference's UOP */

         alpha = *m;
         switch(alpha)
           {
           case 0:
              break;
           case 255:
              alpha = 256 - (*s >> 24);
              *d = *s + MUL_256(alpha, *d);
              break;
           default:
              c = MUL_SYM(alpha, *s);
              alpha = 256 - (c >> 24);
              *d = c + MUL_256(alpha, *d);
              break;
           }
         m++;  s++;  d++; l--;
      },
      { /* A4OP - disabled SSE3 A4OP body verbatim, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);
         __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);

         __m128i c0 = mul_sym_sse3(m0, s0);
         __m128i a0 = sub4_alpha_sse3(c0);
         __m128i r0 = mul_256_sse3(a0, d0);

         r0 = _mm_add_epi32(r0, c0);

         _mm_store_si128((__m128i *)d, r0);

         m += 4; s += 4; d += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);
         __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));

         __m256i c0 = mul_sym_avx2(m0, s0);
         __m256i a0 = sub4_alpha_avx2(c0);
         __m256i r0 = mul_256_avx2(a0, d0);

         r0 = _mm256_add_epi32(r0, c0);

         _mm256_store_si256((__m256i *)d, r0);

         m += 8; s += 8; d += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);
         __m256i m0 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)m));

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));
         __m256i m1 = _mm256_cvtepu8_epi32(_mm_loadl_epi64((__m128i *)(m+8)));

         __m256i c0 = mul_sym_avx2(m0, s0);
         __m256i c1 = mul_sym_avx2(m1, s1);

         __m256i a0 = sub4_alpha_avx2(c0);
         __m256i a1 = sub4_alpha_avx2(c1);

         __m256i r0 = mul_256_avx2(a0, d0);
         __m256i r1 = mul_256_avx2(a1, d1);

         r0 = _mm256_add_epi32(r0, c0);
         r1 = _mm256_add_epi32(r1, c1);

         _mm256_store_si256((__m256i *)d, r0);
         _mm256_store_si256((__m256i *)(d+8), r1);

         m += 16; s += 16; d += 16; l -= 16;
      })
}

#define _op_blend_pas_mas_dp_avx2 _op_blend_p_mas_dp_avx2
#define _op_blend_pan_mas_dp_avx2 _op_blend_pas_mas_dp_avx2

#define _op_blend_p_mas_dpan_avx2 _op_blend_p_mas_dp_avx2
#define _op_blend_pas_mas_dpan_avx2 _op_blend_pas_mas_dp_avx2
#define _op_blend_pan_mas_dpan_avx2 _op_blend_pan_mas_dp_avx2

static void
init_blend_pixel_mask_span_funcs_avx2(void)
{
   /* All 6 slots mirroring init_blend_pixel_mask_span_funcs_sse3's slot
    * list. Bit-exact against C (evas_avx2_ops) - see file header. There is
    * no working SSE3 kernel to compare against
    * (evas_avx2_vs_sse3_ops reports these as avx2-only, as expected). */
   op_blend_span_funcs[SP][SM_AS][SC_N][DP][CPU_AVX2] = _op_blend_p_mas_dp_avx2;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_AVX2] = _op_blend_pas_mas_dp_avx2;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_AVX2] = _op_blend_pan_mas_dp_avx2;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_AVX2] = _op_blend_p_mas_dpan_avx2;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_AVX2] = _op_blend_pas_mas_dpan_avx2;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_AVX2] = _op_blend_pan_mas_dpan_avx2;
}

#endif
