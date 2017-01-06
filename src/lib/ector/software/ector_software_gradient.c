#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <math.h>

#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"
#include "draw.h"

#define GRADIENT_STOPTABLE_SIZE 1024
#define FIXPT_BITS 8
#define FIXPT_SIZE (1<<FIXPT_BITS)

typedef void (*Ector_Radial_Helper_Func)(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data,
                                          float det, float delta_det, float delta_delta_det, float b, float delta_b);

typedef void (*Ector_Linear_Helper_Func)(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data,
                                          int t_fixed, int inc_fixed);

static Ector_Radial_Helper_Func _ector_radial_helper;
static Ector_Linear_Helper_Func _ector_linear_helper;

static inline int
_gradient_clamp(const Ector_Renderer_Software_Gradient_Data *data, int ipos)
{
   int limit;

   if (data->gd->s == EFL_GFX_GRADIENT_SPREAD_REPEAT)
     {
        ipos = ipos % GRADIENT_STOPTABLE_SIZE;
        ipos = ipos < 0 ? GRADIENT_STOPTABLE_SIZE + ipos : ipos;
     }
   else if (data->gd->s == EFL_GFX_GRADIENT_SPREAD_REFLECT)
     {
        limit = GRADIENT_STOPTABLE_SIZE * 2;
        ipos = ipos % limit;
        ipos = ipos < 0 ? limit + ipos : ipos;
        ipos = ipos >= GRADIENT_STOPTABLE_SIZE ? limit - 1 - ipos : ipos;
     }
   else
     {
        if (ipos < 0) ipos = 0;
        else if (ipos >= GRADIENT_STOPTABLE_SIZE)
          ipos = GRADIENT_STOPTABLE_SIZE-1;
     }
   return ipos;
}

static uint32_t
_gradient_pixel_fixed(const Ector_Renderer_Software_Gradient_Data *data, int fixed_pos)
{
   int ipos = (fixed_pos + (FIXPT_SIZE / 2)) >> FIXPT_BITS;

   return data->color_table[_gradient_clamp(data, ipos)];
}

static inline uint32_t
_gradient_pixel(const Ector_Renderer_Software_Gradient_Data *data, float pos)
{
   int ipos = (int)(pos * (GRADIENT_STOPTABLE_SIZE - 1) + (float)(0.5));

   return data->color_table[_gradient_clamp(data, ipos)];
}


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

static void
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

static void
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

typedef double (*BLEND_FUNC)(double progress);

static double
_ease_linear(double t)
{
   return t;
}

static Eina_Bool
_generate_gradient_color_table(Efl_Gfx_Gradient_Stop *gradient_stops, int stop_count, uint32_t *color_table, int size)
{
   int dist, idist, pos = 0, i;
   Eina_Bool alpha = EINA_FALSE;
   Efl_Gfx_Gradient_Stop *curr, *next;
   uint32_t current_color, next_color;
   double delta, t, incr, fpos;

   assert(stop_count > 0);

   curr = gradient_stops;
   if (curr->a != 255) alpha = EINA_TRUE;
   current_color = DRAW_ARGB_JOIN(curr->a, curr->r, curr->g, curr->b);
   incr = 1.0 / (double)size;
   fpos = 1.5 * incr;

   color_table[pos++] = current_color;

   while (fpos <= curr->offset)
     {
        color_table[pos] = color_table[pos - 1];
        pos++;
        fpos += incr;
     }

   for (i = 0; i < stop_count - 1; ++i)
     {
        BLEND_FUNC func;

        curr = (gradient_stops + i);
        next = (gradient_stops + i + 1);
        delta = 1/(next->offset - curr->offset);
        if (next->a != 255) alpha = EINA_TRUE;
        next_color = DRAW_ARGB_JOIN(next->a, next->r, next->g, next->b);
        func = &_ease_linear;
        while (fpos < next->offset && pos < size)
          {
             t = func((fpos - curr->offset) * delta);
             dist = (int)(256 * t);
             idist = 256 - dist;
             color_table[pos] = draw_interpolate_256(current_color, idist, next_color, dist);
             ++pos;
             fpos += incr;
          }
        current_color = next_color;
     }

   for (;pos < size; ++pos)
     color_table[pos] = current_color;

   // Make sure the last color stop is represented at the end of the table
   color_table[size-1] = current_color;
   return alpha;
}


void
update_color_table(Ector_Renderer_Software_Gradient_Data *gdata)
{
   if (gdata->color_table) return;

   gdata->color_table = malloc(GRADIENT_STOPTABLE_SIZE * 4);
   gdata->alpha = _generate_gradient_color_table(gdata->gd->colors, gdata->gd->colors_count,
                                                 gdata->color_table, GRADIENT_STOPTABLE_SIZE);
}

void
destroy_color_table(Ector_Renderer_Software_Gradient_Data *gdata)
{
   if (gdata->color_table)
     {
        free(gdata->color_table);
        gdata->color_table = NULL;
     }
}

static void
_linear_helper_generic(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data,
                       int t_fixed, int inc_fixed)
{
   int i;

   for (i = 0 ; i < length ; i++)
     {
        *buffer++ = _gradient_pixel_fixed(g_data, t_fixed);
        t_fixed += inc_fixed;
     }
}

void
fetch_linear_gradient(uint32_t *buffer, Span_Data *data, int y, int x, int length)
{
   Ector_Renderer_Software_Gradient_Data *g_data = data->gradient;
   float t, inc, rx=0, ry=0;
   uint32_t *end;
   int t_fixed, inc_fixed;

   if (EINA_DBL_EQ(g_data->linear.l, 0.0))
     {
        t = inc = 0;
     }
   else
     {
        rx = data->inv.xy * (y + (float)0.5) + data->inv.xz + data->inv.xx * (x + (float)0.5);
        ry = data->inv.yy * (y + (float)0.5) + data->inv.yz + data->inv.yx * (x + (float)0.5);
        t = g_data->linear.dx*rx + g_data->linear.dy*ry + g_data->linear.off;
        inc = g_data->linear.dx * data->inv.xx + g_data->linear.dx * data->inv.yx;

        t *= (GRADIENT_STOPTABLE_SIZE - 1);
        inc *= (GRADIENT_STOPTABLE_SIZE - 1);
     }

    end = buffer + length;
    if (inc > (float)(-1e-5) && inc < (float)(1e-5))
      {
         draw_memset32(buffer, _gradient_pixel_fixed(g_data, (int)(t * FIXPT_SIZE)), length);
      }
    else
      {
         if (t + inc*length < (float)(INT_MAX >> (FIXPT_BITS + 1)) &&
             t+inc*length > (float)(INT_MIN >> (FIXPT_BITS + 1)))
           {
              // we can use fixed point math
              t_fixed = (int)(t * FIXPT_SIZE);
              inc_fixed = (int)(inc * FIXPT_SIZE);
              _ector_linear_helper(buffer, length, g_data, t_fixed, inc_fixed);
           }
         else
           {
              // we have to fall back to float math
              while (buffer < end)
                {
                   *buffer++ = _gradient_pixel(g_data, t/GRADIENT_STOPTABLE_SIZE);
                   t += inc;
                }
           }
      }
}

static void
_radial_helper_generic(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data, float det,
                       float delta_det, float delta_delta_det, float b, float delta_b)
{
   int i;

   for (i = 0 ; i < length ; i++)
     {
        *buffer++ = _gradient_pixel(g_data, sqrt(det) - b);
        det += delta_det;
        delta_det += delta_delta_det;
        b += delta_b;
     }
}


void
fetch_radial_gradient(uint32_t *buffer, Span_Data *data, int y, int x, int length)
{
   Ector_Renderer_Software_Gradient_Data *g_data = data->gradient;
   float rx, ry, inv_a, delta_rx, delta_ry, b, delta_b, b_delta_b, delta_b_delta_b,
         bb, delta_bb, rxrxryry, delta_rxrxryry, rx_plus_ry, delta_rx_plus_ry, det,
         delta_det, delta_delta_det;

   // avoid division by zero
   if (fabsf(g_data->radial.a) <= 0.00001f)
     {
        draw_memset32(buffer, 0, length);
        return;
     }

   rx = data->inv.xy * (y + (float)0.5) + data->inv.xz + data->inv.xx * (x + (float)0.5);
   ry = data->inv.yy * (y + (float)0.5) + data->inv.yz + data->inv.yx * (x + (float)0.5);

   rx -= g_data->radial.fx;
   ry -= g_data->radial.fy;

   inv_a = 1 / (float)(2 * g_data->radial.a);

   delta_rx = data->inv.xx;
   delta_ry = data->inv.yx;

   b = 2*(g_data->radial.dr*g_data->radial.fradius + rx * g_data->radial.dx + ry * g_data->radial.dy);
   delta_b = 2*(delta_rx * g_data->radial.dx + delta_ry * g_data->radial.dy);
   b_delta_b = 2 * b * delta_b;
   delta_b_delta_b = 2 * delta_b * delta_b;

   bb = b * b;
   delta_bb = delta_b * delta_b;
   b *= inv_a;
   delta_b *= inv_a;

   rxrxryry = rx * rx + ry * ry;
   delta_rxrxryry = delta_rx * delta_rx + delta_ry * delta_ry;
   rx_plus_ry = 2*(rx * delta_rx + ry * delta_ry);
   delta_rx_plus_ry = 2 * delta_rxrxryry;

   inv_a *= inv_a;

   det = (bb - 4 * g_data->radial.a * (g_data->radial.sqrfr - rxrxryry)) * inv_a;
   delta_det = (b_delta_b + delta_bb + 4 * g_data->radial.a * (rx_plus_ry + delta_rxrxryry)) * inv_a;
   delta_delta_det = (delta_b_delta_b + 4 * g_data->radial.a * delta_rx_plus_ry) * inv_a;

   _ector_radial_helper(buffer, length, g_data, det, delta_det, delta_delta_det, b, delta_b);
}

int
ector_software_gradient_init(void)
{
   static int i = 0;
   if (!(i++))
     {
        _ector_radial_helper = _radial_helper_generic;
        _ector_linear_helper = _linear_helper_generic;
#ifdef BUILD_SSE3
        if (eina_cpu_features_get() & EINA_CPU_SSE3)
          {
             _ector_radial_helper = _radial_helper_sse3;
             _ector_linear_helper = _linear_helper_sse3;
          }
#endif
     }
   return i;
}
