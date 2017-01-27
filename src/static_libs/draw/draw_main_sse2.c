#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "draw_private.h"

#ifdef BUILD_SSE3
#include <immintrin.h>

// Each 32bits components of alphaChannel must be in the form 0x00AA00AA
inline static __m128i
v4_byte_mul_sse2(__m128i c, __m128i a)
{
   const __m128i ag_mask = _mm_set1_epi32(0xFF00FF00);
   const __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);

   /* for AG */
   __m128i v_ag = _mm_and_si128(ag_mask, c);
   v_ag = _mm_srli_epi32(v_ag, 8);
   v_ag = _mm_mullo_epi16(a, v_ag);
   v_ag = _mm_and_si128(ag_mask, v_ag);

   /* for RB */
   __m128i v_rb = _mm_and_si128(rb_mask, c);
   v_rb = _mm_mullo_epi16(a, v_rb);
   v_rb = _mm_srli_epi32(v_rb, 8);
   v_rb = _mm_and_si128(rb_mask, v_rb);

   /* combine */
   return _mm_add_epi32(v_ag, v_rb);
}

static inline __m128i
v4_interpolate_color_sse2(__m128i a, __m128i c0, __m128i c1)
{
   const __m128i rb_mask = _mm_set1_epi32(0xFF00FF00);
   const __m128i zero = _mm_setzero_si128();

   __m128i a_l = a;
   __m128i a_h = a;
   a_l = _mm_unpacklo_epi16(a_l, a_l);
   a_h = _mm_unpackhi_epi16(a_h, a_h);

   __m128i a_t = _mm_slli_epi64(a_l, 32);
   __m128i a_t0 = _mm_slli_epi64(a_h, 32);

   a_l = _mm_add_epi32(a_l, a_t);
   a_h = _mm_add_epi32(a_h, a_t0);

   __m128i c0_l = c0;
   __m128i c0_h = c0;

   c0_l = _mm_unpacklo_epi8(c0_l, zero);
   c0_h = _mm_unpackhi_epi8(c0_h, zero);

   __m128i c1_l = c1;
   __m128i c1_h = c1;

   c1_l = _mm_unpacklo_epi8(c1_l, zero);
   c1_h = _mm_unpackhi_epi8(c1_h, zero);

   __m128i cl_sub = _mm_sub_epi16(c0_l, c1_l);
   __m128i ch_sub = _mm_sub_epi16(c0_h, c1_h);

   cl_sub = _mm_mullo_epi16(cl_sub, a_l);
   ch_sub = _mm_mullo_epi16(ch_sub, a_h);

   __m128i c1ls = _mm_slli_epi16(c1_l, 8);
   __m128i c1hs = _mm_slli_epi16(c1_h, 8);

   cl_sub = _mm_add_epi16(cl_sub, c1ls);
   ch_sub = _mm_add_epi16(ch_sub, c1hs);

   cl_sub = _mm_and_si128(cl_sub, rb_mask);
   ch_sub = _mm_and_si128(ch_sub, rb_mask);

   cl_sub = _mm_srli_epi64(cl_sub, 8);
   ch_sub = _mm_srli_epi64(ch_sub, 8);

   cl_sub = _mm_packus_epi16(cl_sub, cl_sub);
   ch_sub = _mm_packus_epi16(ch_sub, ch_sub);

   return  (__m128i) _mm_shuffle_ps( (__m128)cl_sub, (__m128)ch_sub, 0x44);
}

static inline __m128i
v4_mul_color_sse2(__m128i x, __m128i y)
{
   const __m128i zero = _mm_setzero_si128();
   const __m128i sym4_mask = _mm_set_epi32(0x00FF00FF, 0x000000FF, 0x00FF00FF, 0x000000FF);

   __m128i x_l = _mm_unpacklo_epi8(x, zero);
   __m128i x_h = _mm_unpackhi_epi8(x, zero);

   __m128i y_l = _mm_unpacklo_epi8(y, zero);
   __m128i y_h = _mm_unpackhi_epi8(y, zero);

   __m128i r_l = _mm_mullo_epi16(x_l, y_l);
   __m128i r_h = _mm_mullo_epi16(x_h, y_h);

   r_l = _mm_add_epi16(r_l, sym4_mask);
   r_h = _mm_add_epi16(r_h, sym4_mask);

   r_l = _mm_srli_epi16(r_l, 8);
   r_h = _mm_srli_epi16(r_h, 8);

   return  _mm_packus_epi16(r_l, r_h);
}

static inline __m128i
v4_ialpha_sse2(__m128i c)
{
   __m128i a = _mm_srli_epi32(c, 24);

   return _mm_sub_epi32(_mm_set1_epi32(0xff), a);
}

// dest = color + (dest * alpha)
inline static void
comp_func_helper_sse2(uint32_t *dest, int length, uint32_t color, uint32_t alpha)
{
   const __m128i v_color = _mm_set1_epi32(color);
   const __m128i v_a = _mm_set1_epi16(alpha);

   LOOP_ALIGNED_U1_A4(dest, length,
      { /* UOP */
         *dest = color + DRAW_BYTE_MUL(*dest, alpha);
         dest++; length--;
      },
      { /* A4OP */
         __m128i v_dest = _mm_load_si128((__m128i *)dest);

         v_dest = v4_byte_mul_sse2(v_dest, v_a);
         v_dest = _mm_add_epi32(v_dest, v_color);

         _mm_store_si128((__m128i *)dest, v_dest);

         dest += 4; length -= 4;
      })
}

void
comp_func_solid_source_sse2(uint32_t *dest, int length, uint32_t color, uint32_t const_alpha)
{
   if (const_alpha == 255)
     {
        draw_memset32(dest, color, length);
     }
   else
     {
        int ialpha;

        ialpha = 255 - const_alpha;
        color = DRAW_BYTE_MUL(color, const_alpha);
        comp_func_helper_sse2(dest, length, color, ialpha);
     }
}

void
comp_func_solid_source_over_sse2(uint32_t *dest, int length, uint32_t color, uint32_t const_alpha)
{
   int ialpha;

   if (const_alpha != 255)
     color = DRAW_BYTE_MUL(color, const_alpha);
   ialpha = alpha_inverse(color);
   comp_func_helper_sse2(dest, length, color, ialpha);
}

// Load src and dest vector
#define V4_FETCH_SRC_DEST \
  __m128i v_src = _mm_loadu_si128((__m128i *)src); \
  __m128i v_dest = _mm_load_si128((__m128i *)dest);

#define V4_FETCH_SRC \
  __m128i v_src = _mm_loadu_si128((__m128i *)src);

#define V4_STORE_DEST \
  _mm_store_si128((__m128i *)dest, v_src);

#define V4_SRC_DEST_LEN_INC \
  dest += 4; src +=4; length -= 4;

// Multiply src color with color multiplier
#define V4_COLOR_MULTIPLY \
  v_src = v4_mul_color_sse2(v_src, v_color);

// Multiply src color with const_alpha
#define V4_ALPHA_MULTIPLY \
  v_src = v4_byte_mul_sse2(v_src, v_alpha);

// dest = src + dest * sia
#define V4_COMP_OP_SRC_OVER \
  __m128i v_sia = v4_ialpha_sse2(v_src); \
  v_sia = _mm_add_epi32(v_sia, _mm_slli_epi32(v_sia, 16)); \
  v_dest = v4_byte_mul_sse2(v_dest, v_sia); \
  v_src = _mm_add_epi32(v_src, v_dest);

// dest = src + dest * sia
#define V4_COMP_OP_SRC \
  v_src = v4_interpolate_color_sse2(v_alpha, v_src, v_dest);

static void
comp_func_source_sse2(uint32_t *dest, const uint32_t *src, int length, uint32_t color, uint32_t const_alpha)
{
   int ialpha;
   uint32_t src_color;

   if (color == 0xffffffff) // No color multiplier
     {
        if (const_alpha == 255)
          {
             memcpy(dest, src, length * sizeof(uint32_t));
          }
        else
          {
             ialpha = 255 - const_alpha;
             __m128i v_alpha = _mm_set1_epi32(const_alpha);

             LOOP_ALIGNED_U1_A4(dest, length,
               { /* UOP */
                  *dest = draw_interpolate_256(*src, const_alpha, *dest, ialpha);
                  dest++; src++; length--;
               },
               { /* A4OP */
                  V4_FETCH_SRC_DEST
                  V4_COMP_OP_SRC
                  V4_STORE_DEST
                  V4_SRC_DEST_LEN_INC
               })
          }
     }
   else
     {
        __m128i v_color = _mm_set1_epi32(color);

        if (const_alpha == 255)
          {
             LOOP_ALIGNED_U1_A4(dest, length,
               { /* UOP */
                  *dest = DRAW_MUL4_SYM(*src, color);
                  dest++; src++; length--;
               },
               { /* A4OP */
                  V4_FETCH_SRC
                  V4_COLOR_MULTIPLY
                  V4_STORE_DEST
                  V4_SRC_DEST_LEN_INC
               })
          }
        else
          {
             ialpha = 255 - const_alpha;
             __m128i v_alpha = _mm_set1_epi32(const_alpha);

             LOOP_ALIGNED_U1_A4(dest, length,
               { /* UOP */
                  src_color = DRAW_MUL4_SYM(*src, color);
                  *dest = draw_interpolate_256(src_color, const_alpha, *dest, ialpha);
                  dest++; src++; length--;
               },
               { /* A4OP */
                  V4_FETCH_SRC_DEST
                  V4_COLOR_MULTIPLY
                  V4_COMP_OP_SRC
                  V4_STORE_DEST
                  V4_SRC_DEST_LEN_INC
               })
          }
     }
}

static void
comp_func_source_over_sse2(uint32_t *dest, const uint32_t *src, int length, uint32_t color, uint32_t const_alpha)
{
   uint32_t s, sia;

   if (const_alpha != 255)
     color = DRAW_BYTE_MUL(color, const_alpha);

   if (color == 0xffffffff) // No color multiplier
     {
        LOOP_ALIGNED_U1_A4(dest, length,
         { /* UOP */
            s = *src;
            sia = alpha_inverse(s);
            *dest = s + DRAW_BYTE_MUL(*dest, sia);
            dest++; src++; length--;
         },
         { /* A4OP */
            V4_FETCH_SRC_DEST
            V4_COMP_OP_SRC_OVER
            V4_STORE_DEST
            V4_SRC_DEST_LEN_INC
         })
     }
   else
     {
        __m128i v_color = _mm_set1_epi32(color);

        LOOP_ALIGNED_U1_A4(dest, length,
         { /* UOP */
            s = DRAW_MUL4_SYM(*src, color);
            sia = alpha_inverse(s);
            *dest = s + DRAW_BYTE_MUL(*dest, sia);
            dest++; src++; length--;
         },
         { /* A4OP */
            V4_FETCH_SRC_DEST
            V4_COLOR_MULTIPLY
            V4_COMP_OP_SRC_OVER
            V4_STORE_DEST
            V4_SRC_DEST_LEN_INC
         })
     }
}

#endif

void
efl_draw_sse2_init()
{
#ifdef BUILD_SSE3
   if (eina_cpu_features_get() & EINA_CPU_SSE2)
     {
        // update the comp_function table for solid color
        func_for_mode_solid[EFL_GFX_RENDER_OP_COPY] = comp_func_solid_source_sse2;
        func_for_mode_solid[EFL_GFX_RENDER_OP_BLEND] = comp_func_solid_source_over_sse2;

        // update the comp_function table for source data
        func_for_mode[EFL_GFX_RENDER_OP_COPY] = comp_func_source_sse2;
        func_for_mode[EFL_GFX_RENDER_OP_BLEND] = comp_func_source_over_sse2;
      }
#endif
}
