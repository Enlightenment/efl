/* copy color --> dst */

/* Ported from the plain-C reference (op_copy_color_.c). Unlike the pixel
 * group, the C fallback here is a manual byte-store loop (*d = c; d++), not
 * memcpy/memset, so there is a genuine vector opportunity: broadcast the
 * constant color once and store it 8 pixels at a time. Bit-exact by
 * construction (plain stores of the same 32-bit value, no rounding
 * involved). The _rel variant (*d = MUL_SYM(*d>>24, c)) reuses mul_sym_avx2,
 * already established bit-exact against the plain-C MUL_SYM macro (see
 * op_blend_pixel_mask_avx2.c) - same pattern as op_copy_pixel_avx2.c's rel
 * kernel, with the broadcast color standing in for the loaded source
 * pixel. */

#ifdef BUILD_AVX2

static void
_op_copy_c_dp_avx2(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   __m256i cv = _mm256_set1_epi32((int)c);
   int i = 0;

   for (; i + 8 <= l; i += 8)
     _mm256_storeu_si256((__m256i *)(d + i), cv);
   for (; i < l; i++)
     d[i] = c;
}

#define _op_copy_cn_dp_avx2 _op_copy_c_dp_avx2
#define _op_copy_can_dp_avx2 _op_copy_c_dp_avx2
#define _op_copy_caa_dp_avx2 _op_copy_c_dp_avx2

#define _op_copy_c_dpan_avx2 _op_copy_c_dp_avx2
#define _op_copy_cn_dpan_avx2 _op_copy_c_dp_avx2
#define _op_copy_can_dpan_avx2 _op_copy_c_dp_avx2
#define _op_copy_caa_dpan_avx2 _op_copy_c_dp_avx2

static void
init_copy_color_span_funcs_avx2(void)
{
   op_copy_span_funcs[SP_N][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_cn_dp_avx2;
   op_copy_span_funcs[SP_N][SM_N][SC][DP][CPU_AVX2] = _op_copy_c_dp_avx2;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_AVX2] = _op_copy_can_dp_avx2;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_AVX2] = _op_copy_caa_dp_avx2;

   op_copy_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_cn_dpan_avx2;
   op_copy_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_AVX2] = _op_copy_c_dpan_avx2;
   op_copy_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_copy_can_dpan_avx2;
   op_copy_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_copy_caa_dpan_avx2;
}

/*-----*/

/* copy_rel color --> dst */

static void
_op_copy_rel_c_dp_avx2(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c, DATA32 *d, int l) {
   __m256i cv = _mm256_set1_epi32((int)c);
   int i = 0;

   for (; i + 8 <= l; i += 8)
     {
        __m256i d0 = _mm256_loadu_si256((__m256i *)(d + i));
        __m256i a0 = _mm256_srli_epi32(d0, 24);
        __m256i r0 = mul_sym_avx2(a0, cv);

        _mm256_storeu_si256((__m256i *)(d + i), r0);
     }
   for (; i < l; i++)
     d[i] = MUL_SYM(d[i] >> 24, c);
}

#define _op_copy_rel_cn_dp_avx2 _op_copy_rel_c_dp_avx2
#define _op_copy_rel_can_dp_avx2 _op_copy_rel_c_dp_avx2
#define _op_copy_rel_caa_dp_avx2 _op_copy_rel_c_dp_avx2

#define _op_copy_rel_c_dpan_avx2 _op_copy_c_dp_avx2
#define _op_copy_rel_cn_dpan_avx2 _op_copy_cn_dp_avx2
#define _op_copy_rel_can_dpan_avx2 _op_copy_can_dp_avx2
#define _op_copy_rel_caa_dpan_avx2 _op_copy_caa_dp_avx2

static void
init_copy_rel_color_span_funcs_avx2(void)
{
   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_rel_cn_dp_avx2;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP][CPU_AVX2] = _op_copy_rel_c_dp_avx2;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP][CPU_AVX2] = _op_copy_rel_can_dp_avx2;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP][CPU_AVX2] = _op_copy_rel_caa_dp_avx2;

   op_copy_rel_span_funcs[SP_N][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_rel_cn_dpan_avx2;
   op_copy_rel_span_funcs[SP_N][SM_N][SC][DP_AN][CPU_AVX2] = _op_copy_rel_c_dpan_avx2;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AN][DP_AN][CPU_AVX2] = _op_copy_rel_can_dpan_avx2;
   op_copy_rel_span_funcs[SP_N][SM_N][SC_AA][DP_AN][CPU_AVX2] = _op_copy_rel_caa_dpan_avx2;
}

#endif
