#include "ector_software_gradient.h"

#ifdef BUILD_SSE3
void _radial_helper_sse3(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data, float det, float delta_det, float delta_delta_det, float b, float delta_b);
void _linear_helper_sse3(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data, int t, int inc);
#endif

#define GRADIENT_STOPTABLE_SIZE 1024
#define FIXPT_BITS 8
#define FIXPT_SIZE (1<<FIXPT_BITS)

typedef void (*Ector_Radial_Helper_Func)(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data,
                                          float det, float delta_det, float delta_delta_det, float b, float delta_b);

typedef void (*Ector_Linear_Helper_Func)(uint32_t *buffer, int length, Ector_Renderer_Software_Gradient_Data *g_data,
                                          int t_fixed, int inc_fixed);

static Ector_Radial_Helper_Func _ector_radial_helper;
static Ector_Linear_Helper_Func _ector_linear_helper;

static void
_update_color_table(void *data, Ector_Software_Thread *t EINA_UNUSED)
{
   Ector_Renderer_Software_Gradient_Data *gdata = data;

   gdata->color_table = malloc(GRADIENT_STOPTABLE_SIZE * 4);
   gdata->alpha = efl_draw_generate_gradient_color_table(gdata->gd->colors, gdata->gd->colors_count,
                                                         gdata->color_table, GRADIENT_STOPTABLE_SIZE);
}

static void
_done_color_table(void *data)
{
   Ector_Renderer_Software_Gradient_Data *gdata = data;

   gdata->done = EINA_TRUE;
}

void
ector_software_gradient_color_update(Ector_Renderer_Software_Gradient_Data *gdata)
{
   if (!gdata->done)
     {
        ector_software_wait(_update_color_table, _done_color_table, gdata);
        return ;
     }

   if (!gdata->color_table)
     {
        gdata->done = EINA_FALSE;
        ector_software_schedule(_update_color_table, _done_color_table, gdata);
     }
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
         const int vmax = INT_MAX >> (FIXPT_BITS + 1);
         const int vmin = -vmax;
         float v = t + (inc *length);

         if ((v < (float)vmax) && (v > (float)(vmin)))
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
