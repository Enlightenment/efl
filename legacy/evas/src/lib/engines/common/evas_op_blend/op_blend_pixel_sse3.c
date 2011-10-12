/* blend pixel --> dst */

#ifdef BUILD_SSE3

static void
_op_blend_p_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         int alpha = 256 - (*s >> 24);
         *d = *s + MUL_256(alpha, *d);
         s++; d++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i a0 = sub4_alpha_sse3(s0);
         __m128i mul0 = mul_256_sse3(a0, d0);
         d0 = _mm_add_epi32(mul0, s0);

         _mm_store_si128((__m128i *)d, d0);

         s += 4; d += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i a0 = sub4_alpha_sse3(s0);
         __m128i a1 = sub4_alpha_sse3(s1);

         __m128i mul0 = mul_256_sse3(a0, d0);
         __m128i mul1 = mul_256_sse3(a1, d1);

         d0 = _mm_add_epi32(mul0, s0);
         d1 = _mm_add_epi32(mul1, s1);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         s += 8; d += 8; l -= 8;
      })
}

static void
_op_blend_pas_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c __UNUSED__, DATA32 *d, int l) {

   int alpha;

   const __m128i zero = _mm_setzero_si128();

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */
         switch (*s & 0xff000000)
           {
           case 0:
              break;
           case 0xff000000:
              *d = *s;
              break;
           default:
              alpha = 256 - (*s >> 24);
              *d = *s + MUL_256(alpha, *d);
              break;
           }
         s++;  d++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i a0 = sub4_alpha_sse3(s0);
         __m128i mul0 = mul_256_sse3(a0, d0);

         mul0 = _mm_add_epi32(s0, mul0);

         __m128i zmask0 = _mm_cmpeq_epi32(_mm_srli_epi32(s0, 24), zero);
         __m128i imask0  = ~zmask0;

         mul0 = _mm_and_si128(imask0, mul0);
         d0 = _mm_and_si128(zmask0, d0);

         d0 = _mm_add_epi32(mul0, d0);

         _mm_store_si128((__m128i *)d, d0);

        s += 4; d += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i a0 = sub4_alpha_sse3(s0);
         __m128i a1 = sub4_alpha_sse3(s1);

         __m128i mul0 = mul_256_sse3(a0, d0);
         __m128i mul1 = mul_256_sse3(a1, d1);

         mul0 = _mm_add_epi32(s0, mul0);
         mul1 = _mm_add_epi32(s1, mul1);

         __m128i zmask0 = _mm_cmpeq_epi32(_mm_srli_epi32(s0, 24), zero);
         __m128i zmask1 = _mm_cmpeq_epi32(_mm_srli_epi32(s1, 24),  zero);

         __m128i imask0 = ~zmask0;
         __m128i imask1 = ~zmask1;

         mul0 = _mm_and_si128(imask0, mul0);
         d0 = _mm_and_si128(zmask0, d0);

         mul1 = _mm_and_si128(imask1, mul1);
         d1 = _mm_and_si128(zmask1, d1);

         d0 = _mm_add_epi32(mul0, d0);
         d1 = _mm_add_epi32(mul1, d1);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         s += 8; d += 8; l -= 8;
      })
}

#define _op_blend_pan_dp_sse3 NULL

#define _op_blend_p_dpan_sse3 _op_blend_p_dp_sse3
#define _op_blend_pas_dpan_sse3 _op_blend_pas_dp_sse3
#define _op_blend_pan_dpan_sse3 _op_blend_pan_dp_sse3

static void
init_blend_pixel_span_funcs_sse3(void)
{
   op_blend_span_funcs[SP][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_p_dp_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_pas_dp_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_pan_dp_sse3;

   
// FIXME: BUGGY BUGGY Core i5 750 (32bit), 4.5.2 (Ubuntu/Linaro 4.5.2-8ubuntu4), ello (text and rectangle)
//   op_blend_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_p_dpan_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_pas_dpan_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_pan_dpan_sse3;
}

#define _op_blend_pt_p_dp_sse3 NULL

#define _op_blend_pt_pas_dp_sse3 _op_blend_pt_p_dp_sse3
#define _op_blend_pt_pan_dp_sse3 NULL

#define _op_blend_pt_p_dpan_sse3 _op_blend_pt_p_dp_sse3
#define _op_blend_pt_pan_dpan_sse3 _op_blend_pt_pan_dp_sse3
#define _op_blend_pt_pas_dpan_sse3 _op_blend_pt_pas_dp_sse3

static void
init_blend_pixel_pt_funcs_sse3(void)
{
   op_blend_pt_funcs[SP][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_pt_p_dp_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_pt_pas_dp_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_pt_pan_dp_sse3;

   op_blend_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_pt_p_dpan_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_pt_pas_dpan_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_pt_pan_dpan_sse3;
}

/*-----*/

/* blend_rel pixel -> dst */

static void
_op_blend_rel_p_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   const __m128i ones = _mm_set_epi32(1, 1, 1, 1);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         int alpha = 256 - (*s >> 24);
         c = 1 + (*d >> 24);
         *d = MUL_256(c, *s) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /*A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i c0 = _mm_add_epi32(_mm_srli_epi32(d0, 24), ones);
         __m128i a0 = sub4_alpha_sse3(s0);

         d0 = _mm_add_epi32(mul_256_sse3(c0, s0), mul_256_sse3(a0, d0));

         _mm_store_si128((__m128i *)d, d0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128 ((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128 ((__m128i *)(d+4));

         __m128i c0 = _mm_add_epi32(_mm_srli_epi32(d0, 24), ones);
         __m128i c1 = _mm_add_epi32(_mm_srli_epi32(d1, 24), ones);

         __m128i a0 = sub4_alpha_sse3(s0);
         __m128i a1 = sub4_alpha_sse3(s1);

         d0 = _mm_add_epi32(mul_256_sse3(c0, s0), mul_256_sse3(a0, d0));
         d1 = _mm_add_epi32(mul_256_sse3(c1, s1), mul_256_sse3(a1, d1));

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         d += 8; s += 8; l -= 8;
      })
}

static void
_op_blend_rel_pan_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   const __m128i ones = _mm_set_epi32(1, 1, 1, 1);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         c = 1 + (*d >> 24);
         *d++ = MUL_256(c, *s);
         s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i c0 = _mm_add_epi32(_mm_srli_epi32(d0, 24), ones);
         d0 = mul_256_sse3(c0, s0);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128 ((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128 ((__m128i *)(d+4));

         __m128i c0 = _mm_add_epi32(_mm_srli_epi32(d0, 24), ones);
         __m128i c1 = _mm_add_epi32(_mm_srli_epi32(d1, 24), ones);

         d0 = mul_256_sse3(c0, s0);
         d1 = mul_256_sse3(c1, s1);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         d += 8; s += 8; l -= 8;
      })
}

#define _op_blend_rel_pas_dp_sse3 _op_blend_rel_p_dp_sse3

#define _op_blend_rel_p_dpan_sse3 _op_blend_p_dpan_sse3
#define _op_blend_rel_pan_dpan_sse3 _op_blend_pan_dpan_sse3
#define _op_blend_rel_pas_dpan_sse3 _op_blend_pas_dpan_sse3

static void
init_blend_rel_pixel_span_funcs_sse3(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_rel_p_dp_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_rel_pas_dp_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_rel_pan_dp_sse3;

   op_blend_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_p_dpan_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pas_dpan_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pan_dpan_sse3;
}

#define _op_blend_rel_pt_p_dp_sse3 NULL
#define _op_blend_rel_pt_pan_dp_sse3 NULL

#define _op_blend_rel_pt_pas_dp_sse3 _op_blend_rel_pt_p_dp_sse3

#define _op_blend_rel_pt_p_dpan_sse3 _op_blend_pt_p_dpan_sse3
#define _op_blend_rel_pt_pan_dpan_sse3 _op_blend_pt_pan_dpan_sse3
#define _op_blend_rel_pt_pas_dpan_sse3 _op_blend_pt_pas_dpan_sse3

static void
init_blend_rel_pixel_pt_funcs_sse3(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_rel_pt_p_dp_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_rel_pt_pas_dp_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP][CPU_SSE3] = _op_blend_rel_pt_pan_dp_sse3;

   op_blend_rel_pt_funcs[SP][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pt_p_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pas_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pan_dpan_sse3;
}

#endif
