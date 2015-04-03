#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

//Remove
#include <assert.h>

#include <math.h>
#include <float.h>

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"
#include "ector_blend_private.h"


#define GRADIENT_STOPTABLE_SIZE 1024
#define FIXPT_BITS 8
#define FIXPT_SIZE (1<<FIXPT_BITS)


static inline int
_gradient_clamp(const Ector_Renderer_Software_Gradient_Data *data, int ipos)
{
    if (data->gd->s == EFL_GFX_GRADIENT_SPREAD_REPEAT)
      {
         ipos = ipos % GRADIENT_STOPTABLE_SIZE;
         ipos = ipos < 0 ? GRADIENT_STOPTABLE_SIZE + ipos : ipos;
      }
    else if (data->gd->s == EFL_GFX_GRADIENT_SPREAD_REFLECT)
      {
         const int limit = GRADIENT_STOPTABLE_SIZE * 2;
         ipos = ipos % limit;
         ipos = ipos < 0 ? limit + ipos : ipos;
         ipos = ipos >= GRADIENT_STOPTABLE_SIZE ? limit - 1 - ipos : ipos;
      }
    else
      {
         if (ipos < 0)
           ipos = 0;
         else if (ipos >= GRADIENT_STOPTABLE_SIZE)
           ipos = GRADIENT_STOPTABLE_SIZE-1;
      }

    return ipos;
}


static uint
_gradient_pixel_fixed(const Ector_Renderer_Software_Gradient_Data *data, int fixed_pos)
{
   int ipos = (fixed_pos + (FIXPT_SIZE / 2)) >> FIXPT_BITS;
   return data->colorTable[_gradient_clamp(data, ipos)];
}

static inline uint
_gradient_pixel(const Ector_Renderer_Software_Gradient_Data *data, float pos)
{
   int ipos = (int)(pos * (GRADIENT_STOPTABLE_SIZE - 1) + (float)(0.5));
   return data->colorTable[_gradient_clamp(data, ipos)];
}

typedef double (*BLEND_FUNC)(double progress);

static double
_ease_linear(double t)
{
    return t;
}

static void
_generate_gradient_color_table(Efl_Gfx_Gradient_Stop *gradient_stops, int stop_count, uint *colorTable, int size)
{
    int pos = 0;
    Efl_Gfx_Gradient_Stop *curr, *next;
    assert(stop_count > 0);

    curr = gradient_stops;
    uint current_color = ECTOR_ARGB_JOIN(curr->a, curr->r, curr->g, curr->b);
    double incr = 1.0 / (double)size;
    double fpos = 1.5 * incr;

    colorTable[pos++] = current_color;

    while (fpos <= curr->offset)
      {
         colorTable[pos] = colorTable[pos - 1];
         pos++;
         fpos += incr;
      }

    for (int i = 0; i < stop_count - 1; ++i)
      {
         curr = (gradient_stops + i);
         next = (gradient_stops + i + 1);
         double delta = 1/(next->offset - curr->offset);
         uint next_color = ECTOR_ARGB_JOIN(next->a, next->r, next->g, next->b);
         BLEND_FUNC func = &_ease_linear;
         while (fpos < next->offset && pos < size)
           {
              double t = func((fpos - curr->offset) * delta);
              int dist = (int)(256 * t);
              int idist = 256 - dist;
              colorTable[pos] = INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist);
              ++pos;
              fpos += incr;
           }
         current_color = next_color;
      }

    for (;pos < size; ++pos)
      colorTable[pos] = current_color;

    // Make sure the last color stop is represented at the end of the table
    colorTable[size-1] = current_color;
}


void
update_color_table(Ector_Renderer_Software_Gradient_Data *gdata)
{
   if(gdata->colorTable) return;

   gdata->colorTable = malloc(GRADIENT_STOPTABLE_SIZE * 4);
   _generate_gradient_color_table(gdata->gd->colors, gdata->gd->colors_count, gdata->colorTable, GRADIENT_STOPTABLE_SIZE);
}

void
destroy_color_table(Ector_Renderer_Software_Gradient_Data *gdata)
{
   if (gdata->colorTable)
     {
        free(gdata->colorTable);
        gdata->colorTable = NULL;
     }
}


void
fetch_linear_gradient(uint *buffer, Span_Data *data, int y, int x, int length)
{
   Ector_Renderer_Software_Gradient_Data *g_data = data->gradient;
   float t, inc;
   float rx=0, ry=0;

   if (g_data->linear.l == 0)
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

   uint *end = buffer + length;
    if (inc > (float)(-1e-5) && inc < (float)(1e-5))
      {
         _ector_memfill(buffer, _gradient_pixel_fixed(g_data, (int)(t * FIXPT_SIZE)), length);
      }
    else
      {
         if (t + inc*length < (float)(INT_MAX >> (FIXPT_BITS + 1)) &&
             t+inc*length > (float)(INT_MIN >> (FIXPT_BITS + 1)))
           {
              // we can use fixed point math
              int t_fixed = (int)(t * FIXPT_SIZE);
              int inc_fixed = (int)(inc * FIXPT_SIZE);
              // #ifdef BUILD_SSE3
              // if (evas_common_cpu_has_feature(CPU_FEATURE_SSE3)) {
              //     _fetch_linear_sse3(buffer, length, g_data, t_fixed, inc_fixed);
              // } else
              // #endif
              {
                 while (buffer < end)
                   {
                      *buffer++ = _gradient_pixel_fixed(g_data, t_fixed);
                      t_fixed += inc_fixed;
                   }
              }
           }
         else
           {
              // we have to fall back to float math
              while (buffer < end) {
                 *buffer++ = _gradient_pixel(g_data, t/GRADIENT_STOPTABLE_SIZE);
                 t += inc;
              }
           }
      }
}

static void
_radial_helper_generic(uint *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data, float det,
                       float delta_det, float delta_delta_det, float b, float delta_b)
{
    for (int i = 0 ; i < length ; i++)
      {
         *buffer++ = _gradient_pixel(g_data, sqrt(det) - b);
         det += delta_det;
         delta_det += delta_delta_det;
         b += delta_b;
      }
}

void
fetch_radial_gradient(uint *buffer, Span_Data *data, int y, int x, int length)
{
   Ector_Renderer_Software_Gradient_Data *g_data = data->gradient;

   // avoid division by zero
   if (abs(g_data->radial.a) <= 0.00001f)
     {
        _ector_memfill(buffer, 0, length);
        return;
     }

   float rx = data->inv.xy * (y + (float)0.5) + data->inv.xz + data->inv.xx * (x + (float)0.5);
   float ry = data->inv.yy * (y + (float)0.5) + data->inv.yz + data->inv.yx * (x + (float)0.5);

   rx -= g_data->radial.fx;
   ry -= g_data->radial.fy;

   float inv_a = 1 / (float)(2 * g_data->radial.a);

   const float delta_rx = data->inv.xx;
   const float delta_ry = data->inv.yx;

   float b = 2*(g_data->radial.dr*g_data->radial.fradius + rx * g_data->radial.dx + ry * g_data->radial.dy);
   float delta_b = 2*(delta_rx * g_data->radial.dx + delta_ry * g_data->radial.dy);
   const float b_delta_b = 2 * b * delta_b;
   const float delta_b_delta_b = 2 * delta_b * delta_b;

   const float bb = b * b;
   const float delta_bb = delta_b * delta_b;
   b *= inv_a;
   delta_b *= inv_a;

   const float rxrxryry = rx * rx + ry * ry;
   const float delta_rxrxryry = delta_rx * delta_rx + delta_ry * delta_ry;
   const float rx_plus_ry = 2*(rx * delta_rx + ry * delta_ry);
   const float delta_rx_plus_ry = 2 * delta_rxrxryry;

   inv_a *= inv_a;

   float det = (bb - 4 * g_data->radial.a * (g_data->radial.sqrfr - rxrxryry)) * inv_a;
   float delta_det = (b_delta_b + delta_bb + 4 * g_data->radial.a * (rx_plus_ry + delta_rxrxryry)) * inv_a;
   const float delta_delta_det = (delta_b_delta_b + 4 * g_data->radial.a * delta_rx_plus_ry) * inv_a;

   // #ifdef BUILD_SSE3
   //    if (evas_common_cpu_has_feature(CPU_FEATURE_SSE3)) {
   //        _radial_helper_sse3(buffer, length, g_data, det, delta_det, delta_delta_det, b, delta_b);
   //    } else
   // #endif
   { // generic fallback
      _radial_helper_generic(buffer, length, g_data, det, delta_det, delta_delta_det, b, delta_b);
   }
}
