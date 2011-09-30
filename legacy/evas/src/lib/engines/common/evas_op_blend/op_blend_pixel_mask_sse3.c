/* blend pixel x mask --> dst */

#ifdef BUILD_SSE3

static void
_op_blend_p_mas_dp_sse3(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {

   int alpha;

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         alpha = *m;
         c = MUL_SYM(alpha, *s);
         alpha = 256 - (c >> 24);
         *d = c + MUL_256(alpha, *d);
         m++;  s++;  d++; l--;
      },
      { /* A4OP */

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

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);
         __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));
         __m128i m1 = _mm_set_epi32(m[7], m[6], m[5], m[4]);

         __m128i c0 = mul_sym_sse3(m0, s0);
         __m128i c1 = mul_sym_sse3(m1, s1);

         __m128i a0 = sub4_alpha_sse3(c0);
         __m128i a1 = sub4_alpha_sse3(c1);

         __m128i r0 = mul_256_sse3(a0, d0);
         __m128i r1 = mul_256_sse3(a1, d1);

         r0 = _mm_add_epi32(r0, c0);
         r1 = _mm_add_epi32(r1, c1);

         _mm_store_si128((__m128i *)d, r0);
         _mm_store_si128((__m128i *)(d+4), r1);

         m += 8; s += 8; d += 8; l -= 8;
      })
}

static void
_op_blend_pas_mas_dp_sse3(DATA32 *s, DATA8 *m, DATA32 c __UNUSED__, DATA32 *d, int l) {

   const __m128i ones = _mm_set_epi32(1, 1, 1, 1);
   int alpha;

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         alpha = *m;
         switch(alpha)
           {
           case 0:
              break;
           case 255:
              *d = *s;
              break;
           default:
              alpha++;
              *d = INTERP_256(alpha, *s, *d);
              break;
           }
         m++;  s++;  d++; l--;
      },
      {  /*A4OP */

         if ((m[3] | m[2] | m[1] | m[0]) == 0) {
            m += 4; s += 4; d += 4; l -= 4;
            continue;
         }

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);
         __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);

         __m128i zm0 = _mm_cmpeq_epi32(m0, _mm_setzero_si128());

          m0 = _mm_add_epi32(m0, ones);

         __m128i r0 = interp4_256_sse3(m0, s0, d0);

         r0 = _mm_and_si128(~zm0, r0);
         d0 = _mm_and_si128(zm0, d0);

         d0 = _mm_add_epi32(r0, d0);

         _mm_store_si128((__m128i *)d, d0);

         m += 4; s += 4; d += 4; l -= 4;
      },
      { /* A8OP */

         if ((m[7] | m[6] | m[5] | m[4] | m[3] | m[2] | m[1] | m[0]) == 0) {
            m += 8; s += 8; d += 8; l -= 8;
            continue;
         }

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);
         __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));
         __m128i m1 = _mm_set_epi32(m[7], m[6], m[5], m[4]);

         __m128i zm0 = _mm_cmpeq_epi32(m0, _mm_setzero_si128());
         __m128i zm1 = _mm_cmpeq_epi32(m1, _mm_setzero_si128());

         m0 = _mm_add_epi32(m0, ones);
         m1 = _mm_add_epi32(m1, ones);

         __m128i r0 = interp4_256_sse3(m0, s0, d0);
         __m128i r1 = interp4_256_sse3(m1, s1, d1);

         r0 = _mm_and_si128(~zm0, r0);
         d0 = _mm_and_si128(zm0, d0);

         r1 = _mm_and_si128(~zm1, r1);
         d1 = _mm_and_si128(zm1, d1);

         d0 = _mm_add_epi32(d0, r0);
         d1 = _mm_add_epi32(d1, r1);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         m += 8; s += 8; d += 8; l -= 8;
      })
}

#define _op_blend_pan_mas_dp_sse3 _op_blend_pas_mas_dp_sse3

#define _op_blend_p_mas_dpan_sse3 _op_blend_p_mas_dp_sse3
#define _op_blend_pas_mas_dpan_sse3 _op_blend_pas_mas_dp_sse3
#define _op_blend_pan_mas_dpan_sse3 _op_blend_pan_mas_dp_sse3

static void
init_blend_pixel_mask_span_funcs_sse3(void)
{
   op_blend_span_funcs[SP][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_p_mas_dp_sse3;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_pas_mas_dp_sse3;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_pan_mas_dp_sse3;

   op_blend_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_p_mas_dpan_sse3;
   op_blend_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_pas_mas_dpan_sse3;
   op_blend_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_pan_mas_dpan_sse3;
}

#define _op_blend_pt_p_mas_dp_sse3 NULL
#define _op_blend_pt_pan_mas_dp_sse3 NULL

#define _op_blend_pt_pas_mas_dp_sse3 _op_blend_pt_p_mas_dp_sse3

#define _op_blend_pt_p_mas_dpan_sse3 _op_blend_pt_p_mas_dp_sse3
#define _op_blend_pt_pas_mas_dpan_sse3 _op_blend_pt_pas_mas_dp_sse3
#define _op_blend_pt_pan_mas_dpan_sse3 _op_blend_pt_pan_mas_dp_sse3

static void
init_blend_pixel_mask_pt_funcs_sse3(void)
{
   op_blend_pt_funcs[SP][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_pt_p_mas_dp_sse3;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_pt_pas_mas_dp_sse3;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_pt_pan_mas_dp_sse3;

   op_blend_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_pt_p_mas_dpan_sse3;
   op_blend_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_pt_pas_mas_dpan_sse3;
   op_blend_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_pt_pan_mas_dpan_sse3;
}

/*-----*/

/* blend_rel pixel x mask -> dst */

static void
_op_blend_rel_p_mas_dp_sse3(DATA32 *s, DATA8 *m, DATA32 c, DATA32 *d, int l) {

   int alpha;

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         c = MUL_SYM(*m, *s);
         alpha = 256 - (c >> 24);
         *d = MUL_SYM(*d >> 24, c) + MUL_256(alpha, *d);
         d++; m++; s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i c0 = mul_sym_sse3(m0, s0);
         __m128i a0 = sub4_alpha_sse3(c0);

         __m128i l0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), c0);
         __m128i r0 = mul_256_sse3(a0, d0);

         d0 = _mm_add_epi32(l0, r0);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; m += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i m0 = _mm_set_epi32(m[3], m[2], m[1], m[0]);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i m1 = _mm_set_epi32(m[7], m[6], m[5], m[4]);
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i c0 = mul_sym_sse3(m0, s0);
         __m128i c1 = mul_sym_sse3(m1, s1);

         __m128i a0 = sub4_alpha_sse3(c0);
         __m128i a1 = sub4_alpha_sse3(c1);

         __m128i l0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), c0);
         __m128i r0 = mul_256_sse3(a0, d0);

         __m128i l1 = mul_sym_sse3(_mm_srli_epi32(d1, 24), c1);
         __m128i r1 = mul_256_sse3(a1, d1);

         d0 =  _mm_add_epi32(l0, r0);
         d1 = _mm_add_epi32(l1, r1);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         d += 8; m += 8; s += 8; l -= 8;
      })
}

#define _op_blend_rel_pas_mas_dp_sse3 _op_blend_rel_p_mas_dp_sse3
#define _op_blend_rel_pan_mas_dp_sse3 _op_blend_rel_p_mas_dp_sse3

#define _op_blend_rel_p_mas_dpan_sse3 _op_blend_p_mas_dpan_sse3
#define _op_blend_rel_pas_mas_dpan_sse3 _op_blend_pas_mas_dpan_sse3
#define _op_blend_rel_pan_mas_dpan_sse3 _op_blend_pan_mas_dpan_sse3

static void
init_blend_rel_pixel_mask_span_funcs_sse3(void)
{
   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_rel_p_mas_dp_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_rel_pas_mas_dp_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_rel_pan_mas_dp_sse3;

   op_blend_rel_span_funcs[SP][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_p_mas_dpan_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pas_mas_dpan_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pan_mas_dpan_sse3;
}

#define _op_blend_rel_pt_p_mas_dp_sse3 NULL

#define _op_blend_rel_pt_pas_mas_dp_sse3 _op_blend_rel_pt_p_mas_dp_sse3
#define _op_blend_rel_pt_pan_mas_dp_sse3 _op_blend_rel_pt_p_mas_dp_sse3

#define _op_blend_rel_pt_p_mas_dpan_sse3 _op_blend_pt_p_mas_dpan_sse3
#define _op_blend_rel_pt_pas_mas_dpan_sse3 _op_blend_pt_pas_mas_dpan_sse3
#define _op_blend_rel_pt_pan_mas_dpan_sse3 _op_blend_pt_pan_mas_dpan_sse3

static void
init_blend_rel_pixel_mask_pt_funcs_sse3(void)
{
   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_rel_pt_p_mas_dp_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_rel_pt_pas_mas_dp_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP][CPU_SSE3] = _op_blend_rel_pt_pan_mas_dp_sse3;

   op_blend_rel_pt_funcs[SP][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pt_p_mas_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pas_mas_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_AS][SC_N][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pan_mas_dpan_sse3;
}

#endif
