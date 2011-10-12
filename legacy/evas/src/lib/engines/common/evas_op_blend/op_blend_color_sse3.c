/* blend color -> dst */

#ifdef BUILD_SSE3

static void
_op_blend_c_dp_sse3(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   DATA32 a = 256 - (c >> 24);

   const __m128i c_packed = _mm_set_epi32(c, c, c, c);
   const __m128i a_packed = _mm_set_epi32(a, a, a, a);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         *d = c + MUL_256(a, *d);
         d++; l--;
      },
      { /* A4OP */

         __m128i d0 = _mm_load_si128((__m128i *)d);

         d0 = mul_256_sse3(a_packed, d0);
         d0 = _mm_add_epi32(d0, c_packed);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; l -= 4;
      },
      { /* A8OP */

         __m128i d0 = _mm_load_si128((__m128i *)d);
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         d0 = mul_256_sse3(a_packed, d0);
         d1 = mul_256_sse3(a_packed, d1);

         d0 = _mm_add_epi32(d0, c_packed);
         d1 = _mm_add_epi32(d1, c_packed);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         d += 8; l -= 8;
      })
}

#define _op_blend_caa_dp_sse3 _op_blend_c_dp_sse3

#define _op_blend_c_dpan_sse3 _op_blend_c_dp_sse3
#define _op_blend_caa_dpan_sse3 _op_blend_c_dpan_sse3

static void
init_blend_color_span_funcs_sse3(void)
{
   op_blend_span_funcs[SP_N][SM_N][SC][DP][CPU_SSE3] = _op_blend_c_dp_sse3;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_caa_dp_sse3;

// FIXME: BUGGY BUGGY Core i5 750 (32bit), 4.5.2 (Ubuntu/Linaro 4.5.2-8ubuntu4), ello (text and rectangle)
//   op_blend_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_c_dpan_sse3;
   op_blend_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_caa_dpan_sse3;
}

#define _op_blend_pt_c_dp_sse3 NULL
#define _op_blend_pt_caa_dp_sse3 _op_blend_pt_c_dp_sse3

#define _op_blend_pt_c_dpan_sse3 _op_blend_pt_c_dp_sse3
#define _op_blend_pt_caa_dpan_sse3 _op_blend_pt_c_dpan_sse3

#define _op_blend_pt_c_dpas_sse3 _op_blend_pt_c_dp_sse3
#define _op_blend_pt_caa_dpas_sse3 _op_blend_pt_c_dp_sse3

static void
init_blend_color_pt_funcs_sse3(void)
{
   op_blend_pt_funcs[SP_N][SM_N][SC][DP][CPU_SSE3] = _op_blend_pt_c_dp_sse3;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_pt_caa_dp_sse3;

   op_blend_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_pt_c_dpan_sse3;
   op_blend_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_pt_caa_dpan_sse3;
}


/*-----*/

/* blend_rel color -> dst */

static void
_op_blend_rel_c_dp_sse3(DATA32 *s __UNUSED__, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   int alpha = 256 - (c >> 24);

   const __m128i c_packed = _mm_set_epi32(c, c, c, c);
   const __m128i alpha_packed = _mm_set_epi32(alpha, alpha, alpha, alpha);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      {  /* UOP */

         *d = MUL_SYM(*d >> 24, c) + MUL_256(alpha, *d);
         d++; l--;
      },
      { /* A4OP */

         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i mul0 = mul_256_sse3(alpha_packed, d0);
         __m128i sym0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), c_packed);

         d0 = _mm_add_epi32(mul0, sym0);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; l -= 4;
      },
      { /* A8OP */

         __m128i d0 = _mm_load_si128((__m128i *)d);
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i mul0 = mul_256_sse3(alpha_packed, d0);
         __m128i mul1 = mul_256_sse3(alpha_packed, d1);

         __m128i sym0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), c_packed);
         __m128i sym1 = mul_sym_sse3(_mm_srli_epi32(d1, 24), c_packed);

         d0 = _mm_add_epi32(mul0, sym0);
         d1 = _mm_add_epi32(mul1, sym1);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         d += 8; l -= 8;
      })
}

#define _op_blend_rel_caa_dp_sse3 _op_blend_rel_c_dp_sse3
#define _op_blend_rel_c_dpan_sse3 _op_blend_c_dpan_sse3
#define _op_blend_rel_caa_dpan_sse3 _op_blend_caa_dpan_sse3

static void
init_blend_rel_color_span_funcs_sse3(void)
{
   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_c_dp_sse3;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_caa_dp_sse3;

   op_blend_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_c_dpan_sse3;
   op_blend_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_caa_dpan_sse3;
}

#define _op_blend_rel_pt_c_dp_sse3 NULL
#define _op_blend_rel_pt_caa_dp_sse3 _op_blend_rel_pt_c_dp_sse3

#define _op_blend_rel_pt_c_dpan_sse3 _op_blend_pt_c_dpan_sse3
#define _op_blend_rel_pt_caa_dpan_sse3 _op_blend_pt_caa_dpan_sse3

static void
init_blend_rel_color_pt_funcs_sse3(void)
{
   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_pt_c_dp_sse3;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_pt_caa_dp_sse3;

   op_blend_rel_pt_funcs[SP_N][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_pt_c_dpan_sse3;
   op_blend_rel_pt_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_pt_caa_dpan_sse3;
}

#endif
