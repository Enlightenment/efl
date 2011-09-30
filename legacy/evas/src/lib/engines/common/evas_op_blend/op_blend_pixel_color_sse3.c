/* blend pixel x color --> dst */

#ifdef BUILD_SSE3

static void
_op_blend_p_c_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   DATA32 alpha;

   const __m128i c_packed = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         DATA32 sc = MUL4_SYM(c, *s);
         alpha = 256 - (sc >> 24);
         *d = sc + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i sc0 = mul4_sym_sse3(c_packed, s0);
         __m128i a0  = sub4_alpha_sse3(sc0);
         __m128i mul0 = mul_256_sse3(a0, d0);

         d0 = _mm_add_epi32(sc0, mul0);

         _mm_store_si128((__m128i *)d, d0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i sc0 = mul4_sym_sse3(c_packed, s0);
         __m128i sc1 = mul4_sym_sse3(c_packed, s1);

         __m128i a0 = sub4_alpha_sse3(sc0);
         __m128i a1 = sub4_alpha_sse3(sc1);

         __m128i mul0 = mul_256_sse3(a0, d0);
         __m128i mul1 = mul_256_sse3(a1, d1);

         d0 = _mm_add_epi32(sc0, mul0);
         d1 = _mm_add_epi32(sc1, mul1);

         _mm_store_si128((__m128i *)d, d0);
         _mm_store_si128((__m128i *)(d+4), d1);

         d += 8; s += 8; l -= 8;
      })
}

static void
_op_blend_pan_c_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   DATA32 c_a = c & 0xFF000000;
   DATA32 alpha = 256 - (c >> 24);

   const __m128i c_packed = _mm_set_epi32(c, c, c, c);
   const __m128i c_alpha = _mm_set_epi32(c_a, c_a, c_a, c_a);
   const __m128i a0 = _mm_set_epi32(alpha, alpha, alpha, alpha);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         *d = ((c & 0xff000000) + MUL3_SYM(c, *s)) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i r0 = _mm_add_epi32(mul3_sym_sse3(c_packed, s0),
            mul_256_sse3(a0, d0));

         r0 = _mm_add_epi32(r0, c_alpha);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i r0 = _mm_add_epi32(mul3_sym_sse3(c_packed, s0),
            mul_256_sse3(a0, d0));

         __m128i r1 = _mm_add_epi32(mul3_sym_sse3(c_packed, s1),
            mul_256_sse3(a0, d1));

         r0 = _mm_add_epi32(r0, c_alpha);
         r1 = _mm_add_epi32(r1, c_alpha);

         _mm_store_si128((__m128i *)d, r0);
         _mm_store_si128((__m128i *)(d+4), r1);

         d += 8; s += 8; l -= 8;
      })
}

static void
_op_blend_p_can_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   int alpha;
   const __m128i c_packed = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         alpha = 256 - (*s >> 24);
         *d = ((*s & 0xff000000) + MUL3_SYM(c, *s)) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i a0 = sub4_alpha_sse3(s0);

         __m128i r0 = _mm_add_epi32(mul3_sym_sse3(c_packed, s0),
            mul_256_sse3(a0, d0));

         r0 = _mm_add_epi32(r0, _mm_and_si128(s0, A_MASK_SSE3));

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      {
         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i a0 = sub4_alpha_sse3(s0);
         __m128i a1 = sub4_alpha_sse3(s1);

         __m128i r0 = _mm_add_epi32(mul3_sym_sse3(c_packed, s0),
            mul_256_sse3(a0, d0));

         __m128i r1 = _mm_add_epi32(mul3_sym_sse3(c_packed, s1),
            mul_256_sse3(a1, d1));

         r0 = _mm_add_epi32(r0, _mm_and_si128(s0, A_MASK_SSE3));
         r1 = _mm_add_epi32(r1, _mm_and_si128(s1, A_MASK_SSE3));

         _mm_store_si128((__m128i *)d, r0);
         _mm_store_si128((__m128i *)(d+4), r1);

         d += 8; s += 8; l -= 8;
      })
}

static void
_op_blend_pan_can_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   const __m128i c_packed = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         *d++ = 0xff000000 + MUL3_SYM(c, *s);
         s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);

         __m128i r0 = mul3_sym_sse3(c_packed, s0);
         r0 = _mm_add_epi32(r0, A_MASK_SSE3);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      { /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));

         __m128i r0 = mul3_sym_sse3(c_packed, s0);
         __m128i r1 = mul3_sym_sse3(c_packed, s1);

         r0 = _mm_add_epi32(r0, A_MASK_SSE3);
         r1 = _mm_add_epi32(r1, A_MASK_SSE3);

         _mm_store_si128((__m128i *)d, r0);
         _mm_store_si128((__m128i *)(d+4), r1);

         d += 8; s += 8; l -= 8;
      })
}

static void
_op_blend_p_caa_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   int alpha;
   c = 1 + (c & 0xff);
   const __m128i c_packed = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         DATA32 sc = MUL_256(c, *s);
         alpha = 256 - (sc >> 24);
         *d = sc + MUL_256(alpha, *d);
         d++;
         s++;
         l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128 ((__m128i *)d);

         __m128i sc0 = mul_256_sse3(c_packed, s0);
         __m128i a0 = sub4_alpha_sse3(sc0);

         __m128i r0 = _mm_add_epi32(mul_256_sse3(a0, d0), sc0);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      {
         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i sc0 = mul_256_sse3(c_packed, s0);
         __m128i sc1 = mul_256_sse3(c_packed, s1);

         __m128i a0 = sub4_alpha_sse3(sc0);
         __m128i a1 = sub4_alpha_sse3(sc1);

         __m128i r0 = _mm_add_epi32(mul_256_sse3(a0, d0), sc0);
         __m128i r1 = _mm_add_epi32(mul_256_sse3(a1, d1), sc1);

         _mm_store_si128((__m128i *)d, r0);
         _mm_store_si128((__m128i *)(d+4), r1);

         d += 8; s += 8; l -= 8;
      })
}

static void
_op_blend_pan_caa_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   c = 1 + (c & 0xff);
   const __m128i c_packed = _mm_set_epi32(c, c, c,c);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         *d = INTERP_256(c, *s, *d);
         d++; s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i r0 = interp4_256_sse3(c_packed, s0, d0);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      {

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i r0 = interp4_256_sse3(c_packed, s0, d0);
         __m128i r1 = interp4_256_sse3(c_packed, s1, d1);

         _mm_store_si128((__m128i *)d, r0);
         _mm_store_si128((__m128i *)(d+4), r1);

         d += 8; s += 8; l -= 8;
      })
}

#define _op_blend_pas_c_dp_sse3 _op_blend_p_c_dp_sse3
#define _op_blend_pas_can_dp_sse3 _op_blend_p_can_dp_sse3
#define _op_blend_pas_caa_dp_sse3 _op_blend_p_caa_dp_sse3

#define _op_blend_p_c_dpan_sse3 _op_blend_p_c_dp_sse3
#define _op_blend_pas_c_dpan_sse3 _op_blend_pas_c_dp_sse3
#define _op_blend_pan_c_dpan_sse3 _op_blend_pan_c_dp_sse3
#define _op_blend_p_can_dpan_sse3 _op_blend_p_can_dp_sse3
#define _op_blend_pas_can_dpan_sse3 _op_blend_pas_can_dp_sse3
#define _op_blend_pan_can_dpan_sse3 _op_blend_pan_can_dp_sse3
#define _op_blend_p_caa_dpan_sse3 _op_blend_p_caa_dp_sse3
#define _op_blend_pas_caa_dpan_sse3 _op_blend_pas_caa_dp_sse3
#define _op_blend_pan_caa_dpan_sse3 _op_blend_pan_caa_dp_sse3

static void
init_blend_pixel_color_span_funcs_sse3(void)
{
   op_blend_span_funcs[SP][SM_N][SC][DP][CPU_SSE3] = _op_blend_p_c_dp_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP][CPU_SSE3] = _op_blend_pas_c_dp_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP][CPU_SSE3] = _op_blend_pan_c_dp_sse3;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_p_can_dp_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_pas_can_dp_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_pan_can_dp_sse3;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_p_caa_dp_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_pas_caa_dp_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_pan_caa_dp_sse3;

   op_blend_span_funcs[SP][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_p_c_dpan_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_pas_c_dpan_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_pan_c_dpan_sse3;
   op_blend_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_p_can_dpan_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_pas_can_dpan_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_pan_can_dpan_sse3;
   op_blend_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_p_caa_dpan_sse3;
   op_blend_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_pas_caa_dpan_sse3;
   op_blend_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_pan_caa_dpan_sse3;
}

#define _op_blend_pt_p_c_dp_sse3 NULL

#define _op_blend_pt_pas_c_dp_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_pan_c_dp_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_p_can_dp_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_pas_can_dp_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_pan_can_dp_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_p_caa_dp_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_pas_caa_dp_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_pan_caa_dp_sse3 _op_blend_pt_p_c_dp_sse3

#define _op_blend_pt_p_c_dpan_sse3 _op_blend_pt_p_c_dp_sse3
#define _op_blend_pt_pas_c_dpan_sse3 _op_blend_pt_pas_c_dp_sse3
#define _op_blend_pt_pan_c_dpan_sse3 _op_blend_pt_pan_c_dp_sse3
#define _op_blend_pt_p_can_dpan_sse3 _op_blend_pt_p_can_dp_sse3
#define _op_blend_pt_pas_can_dpan_sse3 _op_blend_pt_pas_can_dp_sse3
#define _op_blend_pt_pan_can_dpan_sse3 _op_blend_pt_pan_can_dp_sse3
#define _op_blend_pt_p_caa_dpan_sse3 _op_blend_pt_p_caa_dp_sse3
#define _op_blend_pt_pas_caa_dpan_sse3 _op_blend_pt_pas_caa_dp_sse3
#define _op_blend_pt_pan_caa_dpan_sse3 _op_blend_pt_pan_caa_dp_sse3

static void
init_blend_pixel_color_pt_funcs_sse3(void)
{
   op_blend_pt_funcs[SP][SM_N][SC][DP][CPU_SSE3] = _op_blend_pt_p_c_dp_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP][CPU_SSE3] = _op_blend_pt_pas_c_dp_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP][CPU_SSE3] = _op_blend_pt_pan_c_dp_sse3;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_pt_p_can_dp_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_pt_pas_can_dp_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_pt_pan_can_dp_sse3;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_pt_p_caa_dp_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_pt_pas_caa_dp_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_pt_pan_caa_dp_sse3;

   op_blend_pt_funcs[SP][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_pt_p_c_dpan_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_pt_pas_c_dpan_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_pt_pan_c_dpan_sse3;
   op_blend_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_pt_p_can_dpan_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_pt_pas_can_dpan_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_pt_pan_can_dpan_sse3;
   op_blend_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_pt_p_caa_dpan_sse3;
   op_blend_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_pt_pas_caa_dpan_sse3;
   op_blend_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_pt_pan_caa_dpan_sse3;
}

/*-----*/

/* blend_rel pixel x color -> dst */

static void
_op_blend_rel_p_c_dp_sse3(DATA32 *s, DATA8 *m __UNUSED__, DATA32 c, DATA32 *d, int l) {

   int alpha;

   const __m128i c_packed = _mm_set_epi32(c, c, c, c);

   LOOP_ALIGNED_U1_A48_SSE3(d, l,
      { /* UOP */

         DATA32 sc = MUL4_SYM(c, *s);
         alpha = 256 - (sc >> 24);
         *d = MUL_SYM(*d >> 24, sc) + MUL_256(alpha, *d);
         d++; s++; l--;
      },
      { /* A4OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i sc0 = mul4_sym_sse3(c_packed, s0);
         __m128i a0 = sub4_alpha_sse3(sc0);

         __m128i l0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), sc0);
         __m128i r0 = mul_256_sse3(a0, d0);

         r0 = _mm_add_epi32(l0, r0);

         _mm_store_si128((__m128i *)d, r0);

         d += 4; s += 4; l -= 4;
      },
      {  /* A8OP */

         __m128i s0 = _mm_lddqu_si128((__m128i *)s);
         __m128i d0 = _mm_load_si128((__m128i *)d);

         __m128i s1 = _mm_lddqu_si128((__m128i *)(s+4));
         __m128i d1 = _mm_load_si128((__m128i *)(d+4));

         __m128i sc0 = mul4_sym_sse3(c_packed, s0);
         __m128i sc1 = mul4_sym_sse3(c_packed, s1);

         __m128i a0 = sub4_alpha_sse3(sc0);
         __m128i a1 = sub4_alpha_sse3(sc1);

         __m128i l0 = mul_sym_sse3(_mm_srli_epi32(d0, 24), sc0);
         __m128i r0 = mul_256_sse3(a0, d0);

         __m128i l1 = mul_sym_sse3(_mm_srli_epi32(d1, 24), sc1);
         __m128i r1 = mul_256_sse3(a1, d1);

         r0 = _mm_add_epi32(l0, r0);
         r1 = _mm_add_epi32(l1, r1);

         _mm_store_si128((__m128i *)d, r0);
         _mm_store_si128((__m128i *)(d+4), r1);

         d += 8; s += 8; l -= 8;
      })
}

#define _op_blend_rel_pas_c_dp_sse3 _op_blend_rel_p_c_dp_sse3
#define _op_blend_rel_pan_c_dp_sse3 _op_blend_rel_p_c_dp_sse3
#define _op_blend_rel_p_can_dp_sse3 _op_blend_rel_p_c_dp_sse3
#define _op_blend_rel_pas_can_dp_sse3 _op_blend_rel_p_c_dp_sse3
#define _op_blend_rel_pan_can_dp_sse3 _op_blend_rel_p_c_dp_sse3
#define _op_blend_rel_p_caa_dp_sse3 _op_blend_rel_p_c_dp_sse3
#define _op_blend_rel_pas_caa_dp_sse3 _op_blend_rel_p_c_dp_sse3
#define _op_blend_rel_pan_caa_dp_sse3 _op_blend_rel_p_c_dp_sse3

#define _op_blend_rel_p_c_dpan_sse3 _op_blend_p_c_dpan_sse3
#define _op_blend_rel_pas_c_dpan_sse3 _op_blend_pas_c_dpan_sse3
#define _op_blend_rel_pan_c_dpan_sse3 _op_blend_pan_c_dpan_sse3
#define _op_blend_rel_p_can_dpan_sse3 _op_blend_p_can_dpan_sse3
#define _op_blend_rel_pas_can_dpan_sse3 _op_blend_pas_can_dpan_sse3
#define _op_blend_rel_pan_can_dpan_sse3 _op_blend_pan_can_dpan_sse3
#define _op_blend_rel_p_caa_dpan_sse3 _op_blend_p_caa_dpan_sse3
#define _op_blend_rel_pas_caa_dpan_sse3 _op_blend_pas_caa_dpan_sse3
#define _op_blend_rel_pan_caa_dpan_sse3 _op_blend_pan_caa_dpan_sse3

static void
init_blend_rel_pixel_color_span_funcs_sse3(void)
{
   op_blend_rel_span_funcs[SP][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_p_c_dp_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_pas_c_dp_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_pan_c_dp_sse3;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_rel_p_can_dp_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_rel_pas_can_dp_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_rel_pan_can_dp_sse3;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_p_caa_dp_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_pas_caa_dp_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_pan_caa_dp_sse3;

   op_blend_rel_span_funcs[SP][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_p_c_dpan_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_pas_c_dpan_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_pan_c_dpan_sse3;
   op_blend_rel_span_funcs[SP][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_rel_p_can_dpan_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_rel_pas_can_dpan_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_rel_pan_can_dpan_sse3;
   op_blend_rel_span_funcs[SP][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_p_caa_dpan_sse3;
   op_blend_rel_span_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_pas_caa_dpan_sse3;
   op_blend_rel_span_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_pan_caa_dpan_sse3;
}

#define _op_blend_rel_pt_p_c_dp_sse3 NULL

#define _op_blend_rel_pt_pas_c_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3
#define _op_blend_rel_pt_pan_c_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3
#define _op_blend_rel_pt_p_can_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3
#define _op_blend_rel_pt_pas_can_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3
#define _op_blend_rel_pt_pan_can_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3
#define _op_blend_rel_pt_p_caa_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3
#define _op_blend_rel_pt_pas_caa_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3
#define _op_blend_rel_pt_pan_caa_dp_sse3 _op_blend_rel_pt_p_c_dp_sse3

#define _op_blend_rel_pt_p_c_dpan_sse3 _op_blend_pt_p_c_dpan_sse3
#define _op_blend_rel_pt_pas_c_dpan_sse3 _op_blend_pt_pas_c_dpan_sse3
#define _op_blend_rel_pt_pan_c_dpan_sse3 _op_blend_pt_pan_c_dpan_sse3
#define _op_blend_rel_pt_p_can_dpan_sse3 _op_blend_pt_p_can_dpan_sse3
#define _op_blend_rel_pt_pas_can_dpan_sse3 _op_blend_pt_pas_can_dpan_sse3
#define _op_blend_rel_pt_pan_can_dpan_sse3 _op_blend_pt_pan_can_dpan_sse3
#define _op_blend_rel_pt_p_caa_dpan_sse3 _op_blend_pt_p_caa_dpan_sse3
#define _op_blend_rel_pt_pas_caa_dpan_sse3 _op_blend_pt_pas_caa_dpan_sse3
#define _op_blend_rel_pt_pan_caa_dpan_sse3 _op_blend_pt_pan_caa_dpan_sse3

static void
init_blend_rel_pixel_color_pt_funcs_sse3(void)
{
   op_blend_rel_pt_funcs[SP][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_pt_p_c_dp_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_pt_pas_c_dp_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP][CPU_SSE3] = _op_blend_rel_pt_pan_c_dp_sse3;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_rel_pt_p_can_dp_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_rel_pt_pas_can_dp_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP][CPU_SSE3] = _op_blend_rel_pt_pan_can_dp_sse3;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_pt_p_caa_dp_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_pt_pas_caa_dp_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP][CPU_SSE3] = _op_blend_rel_pt_pan_caa_dp_sse3;

   op_blend_rel_pt_funcs[SP][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_pt_p_c_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pas_c_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pan_c_dpan_sse3;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_rel_pt_p_can_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pas_can_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AN][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pan_can_dpan_sse3;
   op_blend_rel_pt_funcs[SP][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_pt_p_caa_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AS][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pas_caa_dpan_sse3;
   op_blend_rel_pt_funcs[SP_AN][SM_N][SC_AA][DP_AN][CPU_SSE3] = _op_blend_rel_pt_pan_caa_dpan_sse3;
}

#endif
