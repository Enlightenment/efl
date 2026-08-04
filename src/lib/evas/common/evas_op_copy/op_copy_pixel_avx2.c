/* copy pixel --> dst */

/* Ported from the plain-C reference (op_copy_pixel_.c). Copy is bit-exact by
 * definition (no rounding), so there is no divergence excuse here - this
 * must match the C reference exactly, and does: the base kernel is a
 * straight memcpy (identical body to the C fallback, which also calls
 * memcpy - libc's memcpy already dispatches to an AVX2/AVX-512 ifunc on
 * capable hardware, so there is nothing for a hand-rolled AVX2 loop to add
 * here; this slot exists to keep the op-table's CPU_AVX2 tier fully
 * populated for this group, not because it changes codegen). The _rel
 * variant (*d = MUL_SYM(*d>>24, *s)) is a real vector opportunity and uses
 * mul_sym_avx2, already established bit-exact against the plain-C MUL_SYM
 * macro (see op_blend_pixel_mask_avx2.c). */

#ifdef BUILD_AVX2

static void
_op_copy_p_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d, int l) {
   memcpy(d, s, l * sizeof(DATA32));
}

#define _op_copy_pan_dp_avx2 _op_copy_p_dp_avx2
#define _op_copy_pas_dp_avx2 _op_copy_p_dp_avx2

#define _op_copy_p_dpan_avx2 _op_copy_p_dp_avx2
#define _op_copy_pan_dpan_avx2 _op_copy_pan_dp_avx2
#define _op_copy_pas_dpan_avx2 _op_copy_pas_dp_avx2

static void
init_copy_pixel_span_funcs_avx2(void)
{
   op_copy_span_funcs[SP][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_p_dp_avx2;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_pan_dp_avx2;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_pas_dp_avx2;

   op_copy_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_p_dpan_avx2;
   op_copy_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_pan_dpan_avx2;
   op_copy_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_pas_dpan_avx2;
}

/*-----*/

/* copy_rel pixel --> dst */

static void
_op_copy_rel_p_dp_avx2(DATA32 *s, DATA8 *m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d, int l) {
   int i = 0;

   for (; i + 8 <= l; i += 8)
     {
        __m256i s0 = _mm256_loadu_si256((__m256i *)(s + i));
        __m256i d0 = _mm256_loadu_si256((__m256i *)(d + i));
        __m256i a0 = _mm256_srli_epi32(d0, 24);
        __m256i r0 = mul_sym_avx2(a0, s0);

        _mm256_storeu_si256((__m256i *)(d + i), r0);
     }
   for (; i < l; i++)
     d[i] = MUL_SYM(d[i] >> 24, s[i]);
}

#define _op_copy_rel_pas_dp_avx2 _op_copy_rel_p_dp_avx2
#define _op_copy_rel_pan_dp_avx2 _op_copy_rel_p_dp_avx2

#define _op_copy_rel_p_dpan_avx2 _op_copy_p_dpan_avx2
#define _op_copy_rel_pan_dpan_avx2 _op_copy_pan_dpan_avx2
#define _op_copy_rel_pas_dpan_avx2 _op_copy_pas_dpan_avx2

static void
init_copy_rel_pixel_span_funcs_avx2(void)
{
   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_rel_p_dp_avx2;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_rel_pan_dp_avx2;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP][CPU_AVX2] = _op_copy_rel_pas_dp_avx2;

   op_copy_rel_span_funcs[SP][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_rel_p_dpan_avx2;
   op_copy_rel_span_funcs[SP_AN][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_rel_pan_dpan_avx2;
   op_copy_rel_span_funcs[SP_AS][SM_N][SC_N][DP_AN][CPU_AVX2] = _op_copy_rel_pas_dpan_avx2;
}

#endif
