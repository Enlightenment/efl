/* blend pixel x color --> dst */

/* Validated against SSE3, not against the plain-C reference, for the SC/
 * SC_AN kernels (_c_/_can_, built on mul4_sym_avx2/mul3_sym_avx2). SSE3's
 * mul4_sym/mul3_sym round the G channel differently from the plain-C
 * MUL4_SYM/MUL3_SYM macros (1 LSB, most inputs) - a pre-existing property
 * of the shipped SSE3 algorithm, not something introduced by this port.
 * Since AVX2 is meant to be a drop-in replacement for SSE3 (same op table,
 * same slot), the correct target is bit-exact-with-SSE3, and maxdelta=1
 * against plain C for these slots is therefore expected, not a regression.
 *
 * Every kernel in this file uses a 4-wide __m128i stage
 * (LOOP_ALIGNED_U1_A4_A8_A16's A4OP), the SSE3 kernel's A4OP body verbatim.
 * Without it, a 4-7 pixel span ran through AVX2's scalar UOP (exact against
 * C) while the same span on SSE3 ran through SSE3's vector A4OP (carrying
 * the G-channel rounding gap above), so the two tiers disagreed with each
 * other at exactly those lengths even though their 8/16-wide vector
 * arithmetic matches once both sides are actually vectorising - see the
 * LOOP_ALIGNED_U1_A4_A8_A16 comment in evas_blend_ops.h for the mechanism.
 *
 * The SC_AA kernels (mul_256_avx2, interp4_256_avx2) have no rounding gap at
 * any width and are bit-exact against both C and SSE3, with or without the
 * 4-wide stage; it is present for them too, for consistency.
 */

#ifdef BUILD_AVX2

static void
_op_blend_p_c_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   DATA32 alpha;

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         DATA32 sc = MUL4_SYM(c, *s);
         alpha = 256 - (sc >> 24);
         *d = sc + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i sc0 = mul4_sym_sse3(c_packed128, s0);
         __m128i a0  = sub4_alpha_sse3(sc0);
         __m128i mul0 = mul_256_sse3(a0, d0);

         d0 = _mm_add_epi32(sc0, mul0);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i sc0 = mul4_sym_avx2(c_packed, s0);
         __m256i a0  = sub4_alpha_avx2(sc0);
         __m256i mul0 = mul_256_avx2(a0, d0);

         d0 = _mm256_add_epi32(sc0, mul0);

         _mm256_store_si256((__m256i *)d, d0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i sc0 = mul4_sym_avx2(c_packed, s0);
         __m256i sc1 = mul4_sym_avx2(c_packed, s1);

         __m256i a0 = sub4_alpha_avx2(sc0);
         __m256i a1 = sub4_alpha_avx2(sc1);

         __m256i mul0 = mul_256_avx2(a0, d0);
         __m256i mul1 = mul_256_avx2(a1, d1);

         d0 = _mm256_add_epi32(sc0, mul0);
         d1 = _mm256_add_epi32(sc1, mul1);

         _mm256_store_si256((__m256i *)d, d0);
         _mm256_store_si256((__m256i *)(d+8), d1);

         d += 16; s += 16; l -= 16;
      })
}

static void
_op_blend_pan_c_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   DATA32 c_a = c & 0xFF000000;
   DATA32 alpha = 256 - (c >> 24);

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m256i c_alpha = _mm256_set1_epi32(c_a);
   const __m256i a0v = _mm256_set1_epi32(alpha);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);
   const __m128i c_alpha128 = _mm_set_epi32(c_a, c_a, c_a, c_a);
   const __m128i a0v128 = _mm_set_epi32(alpha, alpha, alpha, alpha);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         *d = ((c & 0xff000000) + MUL3_SYM(c, *s)) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i r0 = _mm_add_epi32(mul3_sym_sse3(c_packed128, s0),
            mul_256_sse3(a0v128, d0));

         r0 = _mm_add_epi32(r0, c_alpha128);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i r0 = _mm256_add_epi32(mul3_sym_avx2(c_packed, s0),
            mul_256_avx2(a0v, d0));

         r0 = _mm256_add_epi32(r0, c_alpha);

         _mm256_store_si256((__m256i *)d, r0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i r0 = _mm256_add_epi32(mul3_sym_avx2(c_packed, s0),
            mul_256_avx2(a0v, d0));

         __m256i r1 = _mm256_add_epi32(mul3_sym_avx2(c_packed, s1),
            mul_256_avx2(a0v, d1));

         r0 = _mm256_add_epi32(r0, c_alpha);
         r1 = _mm256_add_epi32(r1, c_alpha);

         _mm256_store_si256((__m256i *)d, r0);
         _mm256_store_si256((__m256i *)(d+8), r1);

         d += 16; s += 16; l -= 16;
      })
}

static void
_op_blend_p_can_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   int alpha;
   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         alpha = 256 - (*s >> 24);
         *d = ((*s & 0xff000000) + MUL3_SYM(c, *s)) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i a0 = sub4_alpha_sse3(s0);

         __m128i r0 = _mm_add_epi32(mul3_sym_sse3(c_packed128, s0),
            mul_256_sse3(a0, d0));

         r0 = _mm_add_epi32(r0, _mm_and_si128(s0, A_MASK_SSE3));

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i a0 = sub4_alpha_avx2(s0);

         __m256i r0 = _mm256_add_epi32(mul3_sym_avx2(c_packed, s0),
            mul_256_avx2(a0, d0));

         r0 = _mm256_add_epi32(r0, _mm256_and_si256(s0, A_MASK_AVX2));

         _mm256_store_si256((__m256i *)d, r0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i a0 = sub4_alpha_avx2(s0);
         __m256i a1 = sub4_alpha_avx2(s1);

         __m256i r0 = _mm256_add_epi32(mul3_sym_avx2(c_packed, s0),
            mul_256_avx2(a0, d0));

         __m256i r1 = _mm256_add_epi32(mul3_sym_avx2(c_packed, s1),
            mul_256_avx2(a1, d1));

         r0 = _mm256_add_epi32(r0, _mm256_and_si256(s0, A_MASK_AVX2));
         r1 = _mm256_add_epi32(r1, _mm256_and_si256(s1, A_MASK_AVX2));

         _mm256_store_si256((__m256i *)d, r0);
         _mm256_store_si256((__m256i *)(d+8), r1);

         d += 16; s += 16; l -= 16;
      })
}

static void
_op_blend_pan_can_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         *d++ = 0xff000000 + MUL3_SYM(c, *s);
         s++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);

         __m128i r0 = mul3_sym_sse3(c_packed128, s0);
         r0 = _mm_add_epi32(r0, A_MASK_SSE3);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);

         __m256i r0 = mul3_sym_avx2(c_packed, s0);
         r0 = _mm256_add_epi32(r0, A_MASK_AVX2);

         _mm256_store_si256((__m256i *)d, r0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));

         __m256i r0 = mul3_sym_avx2(c_packed, s0);
         __m256i r1 = mul3_sym_avx2(c_packed, s1);

         r0 = _mm256_add_epi32(r0, A_MASK_AVX2);
         r1 = _mm256_add_epi32(r1, A_MASK_AVX2);

         _mm256_store_si256((__m256i *)d, r0);
         _mm256_store_si256((__m256i *)(d+8), r1);

         d += 16; s += 16; l -= 16;
      })
}

static void
_op_blend_p_caa_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   int alpha;
   c = 1 + (c & 0xff);
   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         DATA32 sc = MUL_256(c, *s);
         alpha = 256 - (sc >> 24);
         *d = sc + MUL_256(alpha, *d);
         d++;
         s++;
         l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128 ((__m128i *)d);

         __m128i sc0 = mul_256_sse3(c_packed128, s0);
         __m128i a0 = sub4_alpha_sse3(sc0);

         __m128i r0 = _mm_add_epi32(mul_256_sse3(a0, d0), sc0);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256 ((__m256i *)d);

         __m256i sc0 = mul_256_avx2(c_packed, s0);
         __m256i a0 = sub4_alpha_avx2(sc0);

         __m256i r0 = _mm256_add_epi32(mul_256_avx2(a0, d0), sc0);

         _mm256_store_si256((__m256i *)d, r0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i sc0 = mul_256_avx2(c_packed, s0);
         __m256i sc1 = mul_256_avx2(c_packed, s1);

         __m256i a0 = sub4_alpha_avx2(sc0);
         __m256i a1 = sub4_alpha_avx2(sc1);

         __m256i r0 = _mm256_add_epi32(mul_256_avx2(a0, d0), sc0);
         __m256i r1 = _mm256_add_epi32(mul_256_avx2(a1, d1), sc1);

         _mm256_store_si256((__m256i *)d, r0);
         _mm256_store_si256((__m256i *)(d+8), r1);

         d += 16; s += 16; l -= 16;
      })
}

static void
_op_blend_pan_caa_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   c = 1 + (c & 0xff);
   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         *d = INTERP_256(c, *s, *d);
         d++; s++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i r0 = interp4_256_sse3(c_packed128, s0, d0);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i r0 = interp4_256_avx2(c_packed, s0, d0);

         _mm256_store_si256((__m256i *)d, r0);

         d += 8; s += 8; l -= 8;
      },
      { /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i r0 = interp4_256_avx2(c_packed, s0, d0);
         __m256i r1 = interp4_256_avx2(c_packed, s1, d1);

         _mm256_store_si256((__m256i *)d, r0);
         _mm256_store_si256((__m256i *)(d+8), r1);

         d += 16; s += 16; l -= 16;
      })
}

#define _op_blend_pas_c_dp_avx2 _op_blend_p_c_dp_avx2
#define _op_blend_pas_can_dp_avx2 _op_blend_p_can_dp_avx2
#define _op_blend_pas_caa_dp_avx2 _op_blend_p_caa_dp_avx2

#define _op_blend_p_c_dpan_avx2 _op_blend_p_c_dp_avx2
#define _op_blend_pas_c_dpan_avx2 _op_blend_pas_c_dp_avx2
#define _op_blend_pan_c_dpan_avx2 _op_blend_pan_c_dp_avx2
#define _op_blend_p_can_dpan_avx2 _op_blend_p_can_dp_avx2
#define _op_blend_pas_can_dpan_avx2 _op_blend_pas_can_dp_avx2
#define _op_blend_pan_can_dpan_avx2 _op_blend_pan_can_dp_avx2
#define _op_blend_p_caa_dpan_avx2 _op_blend_p_caa_dp_avx2
#define _op_blend_pas_caa_dpan_avx2 _op_blend_pas_caa_dp_avx2
#define _op_blend_pan_caa_dpan_avx2 _op_blend_pan_caa_dp_avx2

static void
init_blend_pixel_color_span_funcs_avx2(void)
{
   /* All 18 slots, mirroring init_blend_pixel_color_span_funcs_sse3 exactly.
    * Bit-exact against SSE3 at every length (evas_avx2_vs_sse3_ops: 0 diff,
    * maxdelta=0). The SC/SC_AN slots legitimately show maxdelta=1 against
    * plain C (evas_avx2_ops) - see the file header. */
   op_blend_span_funcs[SP][SM_N][SC][DP][CPU_AVX2] = _op_blend_p_c_dp_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP][CPU_AVX2] = _op_blend_pas_c_dp_avx2;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP][CPU_AVX2] = _op_blend_pan_c_dp_avx2;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP][CPU_AVX2] = _op_blend_p_can_dp_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_AVX2] = _op_blend_pas_can_dp_avx2;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_AVX2] = _op_blend_pan_can_dp_avx2;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_p_caa_dp_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_pas_caa_dp_avx2;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_pan_caa_dp_avx2;

   op_blend_span_funcs[SP][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_p_c_dpan_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_pas_c_dpan_avx2;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_pan_c_dpan_avx2;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_blend_p_can_dpan_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_blend_pas_can_dpan_avx2;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_blend_pan_can_dpan_avx2;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_p_caa_dpan_avx2;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_pas_caa_dpan_avx2;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_pan_caa_dpan_avx2;
}

/*-----*/

/* blend_rel pixel x color -> dst */

static void
_op_blend_rel_p_c_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {

   int alpha;

   const __m256i c_packed = _mm256_set1_epi32(c);
   const __m128i c_packed128 = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A4_A8_A16(d, l,
      { /* UOP */

         DATA32 sc = MUL4_SYM(c, *s);
         alpha = 256 - (sc >> 24);
         *d = MUL_SYM(*d >> 24, sc) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP - verbatim SSE3 A4OP body, see file header */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i sc0 = mul4_sym_sse3(c_packed128, s0);
         __m128i a0 = sub4_alpha_sse3(sc0);

         __m128i l0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), sc0);
         __m128i r0 = mul_256_sse3(a0, d0);

         r0 = _mm_add_epi32(l0, r0);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      {  /* A8OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i sc0 = mul4_sym_avx2(c_packed, s0);
         __m256i a0 = sub4_alpha_avx2(sc0);

         __m256i l0 = mul_sym_avx2(_mm256_srli_epi32(d0, 24), sc0);
         __m256i r0 = mul_256_avx2(a0, d0);

         r0 = _mm256_add_epi32(l0, r0);

         _mm256_store_si256((__m256i *)d, r0);

         d += 8; s += 8; l -= 8;
      },
      {  /* A16OP */

         __m256i s0 = _mm256_loadu_si256((__m256i *)s);
         __m256i d0 = _mm256_load_si256((__m256i *)d);

         __m256i s1 = _mm256_loadu_si256((__m256i *)(s+8));
         __m256i d1 = _mm256_load_si256((__m256i *)(d+8));

         __m256i sc0 = mul4_sym_avx2(c_packed, s0);
         __m256i sc1 = mul4_sym_avx2(c_packed, s1);

         __m256i a0 = sub4_alpha_avx2(sc0);
         __m256i a1 = sub4_alpha_avx2(sc1);

         __m256i l0 = mul_sym_avx2(_mm256_srli_epi32(d0, 24), sc0);
         __m256i r0 = mul_256_avx2(a0, d0);

         __m256i l1 = mul_sym_avx2(_mm256_srli_epi32(d1, 24), sc1);
         __m256i r1 = mul_256_avx2(a1, d1);

         r0 = _mm256_add_epi32(l0, r0);
         r1 = _mm256_add_epi32(l1, r1);

         _mm256_store_si256((__m256i *)d, r0);
         _mm256_store_si256((__m256i *)(d+8), r1);

         d += 16; s += 16; l -= 16;
      })
}

#define _op_blend_rel_pas_c_dp_avx2 _op_blend_rel_p_c_dp_avx2
#define _op_blend_rel_pan_c_dp_avx2 _op_blend_rel_p_c_dp_avx2
#define _op_blend_rel_p_can_dp_avx2 _op_blend_rel_p_c_dp_avx2
#define _op_blend_rel_pas_can_dp_avx2 _op_blend_rel_p_c_dp_avx2
#define _op_blend_rel_pan_can_dp_avx2 _op_blend_rel_p_c_dp_avx2
#define _op_blend_rel_p_caa_dp_avx2 _op_blend_rel_p_c_dp_avx2
#define _op_blend_rel_pas_caa_dp_avx2 _op_blend_rel_p_c_dp_avx2
#define _op_blend_rel_pan_caa_dp_avx2 _op_blend_rel_p_c_dp_avx2

#define _op_blend_rel_p_c_dpan_avx2 _op_blend_p_c_dpan_avx2
#define _op_blend_rel_pas_c_dpan_avx2 _op_blend_pas_c_dpan_avx2
#define _op_blend_rel_pan_c_dpan_avx2 _op_blend_pan_c_dpan_avx2
#define _op_blend_rel_p_can_dpan_avx2 _op_blend_p_can_dpan_avx2
#define _op_blend_rel_pas_can_dpan_avx2 _op_blend_pas_can_dpan_avx2
#define _op_blend_rel_pan_can_dpan_avx2 _op_blend_pan_can_dpan_avx2
#define _op_blend_rel_p_caa_dpan_avx2 _op_blend_p_caa_dpan_avx2
#define _op_blend_rel_pas_caa_dpan_avx2 _op_blend_pas_caa_dpan_avx2
#define _op_blend_rel_pan_caa_dpan_avx2 _op_blend_pan_caa_dpan_avx2

static void
init_blend_rel_pixel_color_span_funcs_avx2(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC][DP][CPU_AVX2] = _op_blend_rel_p_c_dp_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_AVX2] = _op_blend_rel_pas_c_dp_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_AVX2] = _op_blend_rel_pan_c_dp_avx2;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_AVX2] = _op_blend_rel_p_can_dp_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_AVX2] = _op_blend_rel_pas_can_dp_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_AVX2] = _op_blend_rel_pan_can_dp_avx2;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_rel_p_caa_dp_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_rel_pas_caa_dp_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_AVX2] = _op_blend_rel_pan_caa_dp_avx2;

   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_rel_p_c_dpan_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_rel_pas_c_dpan_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_AVX2] = _op_blend_rel_pan_c_dpan_avx2;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_blend_rel_p_can_dpan_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_blend_rel_pas_can_dpan_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_blend_rel_pan_can_dpan_avx2;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_rel_p_caa_dpan_avx2;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_rel_pas_caa_dpan_avx2;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_blend_rel_pan_caa_dpan_avx2;
}

#endif
