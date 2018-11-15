#include "ector_software_gradient.h"

#ifdef BUILD_SSE3
#include <immintrin.h>

#define GRADIENT_STOPTABLE_SIZE_SHIFT 10
typedef union { __m128i v; int i[4];}  vec4_i;
typedef union { __m128 v; float f[4];} vec4_f;

#define FETCH_CLAMP_INIT_F \
  __m128 v_min = _mm_set1_ps(0.0f); \
  __m128 v_max = _mm_set1_ps((float)(GRADIENT_STOPTABLE_SIZE-1)); \
  __m128 v_halff = _mm_set1_ps(0.5f); \
  __m128i v_repeat_mask = _mm_set1_epi32(~((uint32_t)(0xffffff) << GRADIENT_STOPTABLE_SIZE_SHIFT)); \
  __m128i v_reflect_mask = _mm_set1_epi32(~((uint32_t)(0xffffff) << (GRADIENT_STOPTABLE_SIZE_SHIFT+1))); \
  __m128i v_reflect_limit = _mm_set1_epi32(2 * GRADIENT_STOPTABLE_SIZE - 1);

#define FETCH_CLAMP_REPEAT_F \
  vec4_i index_vec; \
  index_vec.v = _mm_and_si128(v_repeat_mask, _mm_cvttps_epi32(v_index));

#define FETCH_CLAMP_REFLECT_F \
  vec4_i index_vec; \
  __m128i v_index_i = _mm_and_si128(v_reflect_mask, _mm_cvttps_epi32(v_index)); \
  __m128i v_index_i_inv = _mm_sub_epi32(v_reflect_limit, v_index_i); \
  index_vec.v = _mm_min_epi16(v_index_i, v_index_i_inv);

#define FETCH_CLAMP_PAD_F \
  vec4_i index_vec; \
  index_vec.v = _mm_cvttps_epi32(_mm_min_ps(v_max, _mm_max_ps(v_min, v_index)));

#define FETCH_EPILOGUE_CPY \
  *buffer++ = g_data->color_table[index_vec.i[0]]; \
  *buffer++ = g_data->color_table[index_vec.i[1]]; \
  *buffer++ = g_data->color_table[index_vec.i[2]]; \
  *buffer++ = g_data->color_table[index_vec.i[3]]; \
}

static void
loop_break(unsigned int *buffer, int length, int *lprealign, int *lby4 , int *lremaining)
{
   int l1=0, l2=0, l3=0;

   while ((uintptr_t)buffer & 0xF)
     buffer++ , l1++;

   if(length <= l1)
     {
        l1 = length;
     }
   else
     {
        l3 = (length - l1) % 4;
        l2 = length - l1 - l3 ;
     }

   *lprealign = l1;
   *lby4 = l2;
   *lremaining = l3;
}

void
_radial_helper_sse3(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data,
                    float det, float delta_det, float delta_delta_det, float b, float delta_b)
{
   int lprealign, lby4, lremaining, i;
   vec4_f det_vec;
   vec4_f delta_det4_vec;
   vec4_f b_vec;
   __m128 v_delta_delta_det16;
   __m128 v_delta_delta_det6;
   __m128 v_delta_b4;

   loop_break(buffer, length, &lprealign, &lby4, &lremaining);

   // prealign loop
   for (i = 0 ; i < lprealign ; i++)
     {
        *buffer++ = _gradient_pixel(g_data, sqrt(det) - b);
        det += delta_det;
        delta_det += delta_delta_det;
        b += delta_b;
     }

   // lby4 16byte align loop
   for (i = 0; i < 4; ++i)
     {
        det_vec.f[i] = det;
        delta_det4_vec.f[i] = 4 * delta_det;
        b_vec.f[i] = b;

        det += delta_det;
        delta_det += delta_delta_det;
        b += delta_b;
     }

   v_delta_delta_det16 = _mm_set1_ps(16 * delta_delta_det);
   v_delta_delta_det6 = _mm_set1_ps(6 * delta_delta_det);
   v_delta_b4 = _mm_set1_ps(4 * delta_b);

#define FETCH_RADIAL_PROLOGUE                                           \
   for (i = 0 ; i < lby4 ; i+=4) {                                      \
      __m128 v_index_local = _mm_sub_ps(_mm_sqrt_ps(det_vec.v), b_vec.v); \
      __m128 v_index = _mm_add_ps(_mm_mul_ps(v_index_local, v_max), v_halff); \
      det_vec.v = _mm_add_ps(_mm_add_ps(det_vec.v, delta_det4_vec.v), v_delta_delta_det6); \
      delta_det4_vec.v = _mm_add_ps(delta_det4_vec.v, v_delta_delta_det16); \
      b_vec.v = _mm_add_ps(b_vec.v, v_delta_b4);

#define FETCH_RADIAL_LOOP(FETCH_CLAMP) \
   FETCH_RADIAL_PROLOGUE;              \
   FETCH_CLAMP;                        \
   FETCH_EPILOGUE_CPY;

   FETCH_CLAMP_INIT_F;
   switch (g_data->gd->s)
     {
      case EFL_GFX_GRADIENT_SPREAD_REPEAT:
         FETCH_RADIAL_LOOP(FETCH_CLAMP_REPEAT_F);
         break;
      case EFL_GFX_GRADIENT_SPREAD_REFLECT:
         FETCH_RADIAL_LOOP( FETCH_CLAMP_REFLECT_F);
         break;
      default:
         FETCH_RADIAL_LOOP(FETCH_CLAMP_PAD_F);
         break;
     }

   // remaining loop
   for (i = 0 ; i < lremaining ; i++)
     *buffer++ = _gradient_pixel(g_data, sqrt(det_vec.f[i]) - b_vec.f[i]);
}

void
_linear_helper_sse3(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data, int t, int inc)
{
   int lprealign, lby4, lremaining, i;
   vec4_i t_vec;
   __m128i v_inc;
   __m128i v_fxtpt_size;
   __m128i v_min;
   __m128i v_max;
   __m128i v_repeat_mask;
   __m128i v_reflect_mask;
   __m128i v_reflect_limit;

   loop_break(buffer, length, &lprealign, &lby4, &lremaining);

   // prealign loop
   for (i = 0 ; i < lprealign ; i++)
     {
        *buffer++ = _gradient_pixel_fixed(g_data, t);
        t += inc;
     }

   // lby4 16byte align loop
   for (i = 0; i < 4; ++i)
     {
        t_vec.i[i] = t;
        t += inc;
     }

   v_inc = _mm_set1_epi32(4 * inc);
   v_fxtpt_size = _mm_set1_epi32(FIXPT_SIZE * 0.5);

   v_min = _mm_set1_epi32(0);
   v_max = _mm_set1_epi32((GRADIENT_STOPTABLE_SIZE - 1));

   v_repeat_mask = _mm_set1_epi32(~((uint32_t)(0xffffff) << GRADIENT_STOPTABLE_SIZE_SHIFT));
   v_reflect_mask = _mm_set1_epi32(~((uint32_t)(0xffffff) << (GRADIENT_STOPTABLE_SIZE_SHIFT + 1)));

   v_reflect_limit = _mm_set1_epi32(2 * GRADIENT_STOPTABLE_SIZE - 1);

#define FETCH_LINEAR_LOOP_PROLOGUE                                      \
   for (i = 0 ; i < lby4 ; i+=4) {                                      \
      vec4_i index_vec;                                                 \
      __m128i v_index;                                                  \
      v_index =  _mm_srai_epi32(_mm_add_epi32(t_vec.v, v_fxtpt_size), FIXPT_BITS); \
      t_vec.v = _mm_add_epi32(t_vec.v, v_inc);

#define FETCH_LINEAR_LOOP_CLAMP_REPEAT                  \
   index_vec.v = _mm_and_si128(v_repeat_mask, v_index);

#define FETCH_LINEAR_LOOP_CLAMP_REFLECT                                 \
   __m128i v_index_i = _mm_and_si128(v_reflect_mask, v_index);          \
   __m128i v_index_i_inv = _mm_sub_epi32(v_reflect_limit, v_index_i);   \
   index_vec.v = _mm_min_epi16(v_index_i, v_index_i_inv);

#define FETCH_LINEAR_LOOP_CLAMP_PAD                                     \
   index_vec.v = _mm_min_epi16(v_max, _mm_max_epi16(v_min, v_index));

#define FETCH_LINEAR_LOOP(FETCH_LINEAR_LOOP_CLAMP)      \
   FETCH_LINEAR_LOOP_PROLOGUE;                          \
   FETCH_LINEAR_LOOP_CLAMP;                             \
   FETCH_EPILOGUE_CPY;

   switch (g_data->gd->s)
     {
      case EFL_GFX_GRADIENT_SPREAD_REPEAT:
         FETCH_LINEAR_LOOP(FETCH_LINEAR_LOOP_CLAMP_REPEAT);
         break;
      case EFL_GFX_GRADIENT_SPREAD_REFLECT:
         FETCH_LINEAR_LOOP(FETCH_LINEAR_LOOP_CLAMP_REFLECT);
         break;
      default:
         FETCH_LINEAR_LOOP(FETCH_LINEAR_LOOP_CLAMP_PAD);
         break;
     }

   // remaining loop
   for (i = 0 ; i < lremaining ; i++)
     *buffer++ = _gradient_pixel_fixed(g_data, t_vec.i[i]);
}

#endif
