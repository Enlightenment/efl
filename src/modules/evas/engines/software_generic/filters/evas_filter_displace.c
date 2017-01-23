#include "evas_engine_filter.h"

static void
_filter_displace_cpu_alpha_do(int w, int h, int map_w, int map_h, int intensity,
                              uint8_t *src, uint8_t *dst, uint32_t *map_start,
                              Eina_Bool stretch, Eina_Bool smooth,
                              Eina_Bool blend)
{
   int x, y, map_x, map_y;
   const int dx = RED;
   const int dy = GREEN;
   uint8_t *map;

   // FIXME: Add stride support

   for (y = 0, map_y = 0; y < h; y++, map_y++)
     {
        if (map_y >= map_h) map_y = 0;
        map = (uint8_t *) (map_start + map_y * map_w);

        for (x = 0, map_x = 0; x < w;
             x++, dst++, src++, map_x++, map += sizeof(uint32_t))
          {
             int offx = 0, offy = 0, offx_dec = 0, offy_dec = 0, val = 0;
             Eina_Bool out = 0;

             // wrap around (x)
             if (map_x >= map_w)
               {
                  map_x = 0;
                  map = (uint8_t *) (map_start + map_y * map_w);
               }

             // x
             val = ((int) map[dx] - 128) * intensity;
             offx = val >> 7;
             offx_dec = val & 0x7f;
             if ((x + offx) < 0) { offx = -x; out = 1; }
             if ((x + offx + 1) >= w) { offx = w - x - 2; out = 1; }

             // y
             val = ((int) map[dy] - 128) * intensity;
             offy = val >> 7;
             offy_dec = val & 0x7f;
             if ((y + offy) < 0) { offy = -y; out = 1; }
             if ((y + offy + 1) >= h) { offy = h - y - 2; out = 1; }

             // get value
             if (out && !stretch)
               val = 0;
             else
               {
                  if (!smooth)
                    val = src[offx + offy * w];
                  else
                    {
                       val  = src[offx + offy * w] * (128 - offx_dec) * (128 - offy_dec);
                       val += src[offx + 1 + offy * w] * offx_dec * (128 - offy_dec);
                       val += src[offx + (offy + 1) * w] * (128 - offx_dec) * offy_dec;
                       val += src[offx + 1 + (offy + 1) * w] * offx_dec * offy_dec;
                       val = val >> 14; // <=> *dst = val / (128 * 128)
                    }
               }

             // apply alpha
             if (map[ALPHA] != 255)
               val = (val * map[ALPHA]) / 255;

             // write to dest
             if (blend)
               *dst = (*dst * (255 - val)) / 255 + val;
             else
               *dst = val;
          }
     }
}

static void
_filter_displace_cpu_rgba_do(int w, int h, int map_w, int map_h, int intensity,
                             uint32_t *src, uint32_t *dst, uint32_t *map_start,
                             Eina_Bool stretch, Eina_Bool smooth,
                             Eina_Bool blend)
{
   int x, y, map_x, map_y;
   const int dx = RED;
   const int dy = GREEN;
   Eina_Bool unpremul = EINA_FALSE;
   uint8_t *map;

   for (y = 0, map_y = 0; y < h; y++, map_y++)
     {
        if (map_y >= map_h) map_y = 0;
        map = (uint8_t *) (map_start + map_y * map_w);

        for (x = 0, map_x = 0; x < w;
             x++, dst++, src++, map_x++, map += sizeof(uint32_t))
          {
             int offx = 0, offy = 0, offx_dec = 0, offy_dec = 0, val = 0;
             uint32_t col = 0;
             Eina_Bool out = 0;

             // wrap (x)
             if (map_x >= map_w)
               {
                  map_x = 0;
                  map = (uint8_t *) (map_start + map_y * map_w);
               }

             if (!map[ALPHA]) continue;
             if (!unpremul && map[ALPHA] != 0xFF)
               {
                  unpremul = EINA_TRUE;
                  evas_data_argb_unpremul(map_start, map_w * map_h);
               }

             // x
             val = ((int) map[dx] - 128) * intensity;
             offx = val >> 7;
             offx_dec = val & 0x7f;
             if ((x + offx) < 0) { offx = -x; out = 1; }
             if ((x + offx + 1) >= w) { offx = w - x - 2; out = 1; }

             // y
             val = ((int) map[dy] - 128) * intensity;
             offy = val >> 7;
             offy_dec = val & 0x7f;
             if ((y + offy) < 0) { offy = -y; out = 1; }
             if ((y + offy + 1) >= h) { offy = h - y - 2; out = 1; }

             // get value
             if (out && !stretch)
               col = A_VAL(src + offx + offy * w) << (ALPHA * 8);
             else if (!smooth)
               col = src[offx + offy * w];
             else
               {
                  int R, G, B, A;
                  uint32_t s00, s01, s10, s11; // indexes represent x,y
                  int mul00, mul01, mul10, mul11;

                  mul00 = (128 - offx_dec) * (128 - offy_dec);
                  mul01 = (128 - offx_dec) * offy_dec;
                  mul10 = offx_dec * (128 - offy_dec);
                  mul11 = offx_dec * offy_dec;

                  s00 = src[offx + offy * w];
                  s01 = src[offx + (offy + 1) * w];
                  s10 = src[offx + 1 + offy * w];
                  s11 = src[offx + 1 + (offy + 1) * w];

                  A = (ALPHA_OF(s00) * mul00) + (ALPHA_OF(s10) * mul10)
                        + (ALPHA_OF(s01) * mul01) + (ALPHA_OF(s11) * mul11);

                  R = (RED_OF(s00) * mul00) + (RED_OF(s10) * mul10)
                        + (RED_OF(s01) * mul01) + (RED_OF(s11) * mul11);

                  G = (GREEN_OF(s00) * mul00) + (GREEN_OF(s10) * mul10)
                        + (GREEN_OF(s01) * mul01) + (GREEN_OF(s11) * mul11);

                  B = (BLUE_OF(s00) * mul00) + (BLUE_OF(s10) * mul10)
                        + (BLUE_OF(s01) * mul01) + (BLUE_OF(s11) * mul11);

                  A >>= 14;
                  R >>= 14;
                  G >>= 14;
                  B >>= 14;

                  col = ARGB_JOIN(A, R, G, B);
               }

             if (map[ALPHA] != 0xFF)
               col = MUL_256(map[ALPHA], col);

             if (blend)
               {
                  uint32_t a = 256 - ALPHA_OF(col);
                  *dst = col + MUL_256(a, *dst);
               }
             else
               *dst = col;
          }
     }

   if (unpremul)
     evas_data_argb_premul(map_start, map_w * map_h);
}

/**
 * Apply distortion map on alpha image
 * input:  alpha
 * output: alpha
 * map:    rg+a (rgba)
 */
static Eina_Bool
_filter_displace_cpu_alpha(Evas_Filter_Command *cmd)
{
   unsigned int src_len, src_stride, map_len, map_stride, dst_len, dst_stride;
   int w, h, map_w, map_h, intensity;
   uint8_t *dst, *src;
   uint32_t *map_start;
   Eina_Bool stretch, smooth, blend;
   Evas_Filter_Buffer *map_fb;
   Eina_Bool ret = EINA_FALSE;

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w == cmd->output->w, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(h == cmd->output->h, EINA_FALSE);

   src = _buffer_map_all(cmd->input->buffer, &src_len, E_READ, E_ALPHA, &src_stride);
   dst = _buffer_map_all(cmd->output->buffer, &dst_len, E_READ | E_WRITE, E_ALPHA, &dst_stride);
   stretch = cmd->displacement.flags & EVAS_FILTER_DISPLACE_STRETCH;
   smooth = cmd->displacement.flags & EVAS_FILTER_DISPLACE_LINEAR;
   map_w = cmd->mask->w;
   map_h = cmd->mask->h;
   intensity = cmd->displacement.intensity;
   blend = (cmd->draw.rop == EFL_GFX_RENDER_OP_BLEND);

   // Stretch if necessary.
   if ((map_w != w || map_h != h) && (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_XY))
     {
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
          map_w = w;
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          map_h = h;

        BUFFERS_LOCK();
        map_fb = cmd->ctx->buffer_scaled_get(cmd->ctx, cmd->mask, map_w, map_h);
        BUFFERS_UNLOCK();

        EINA_SAFETY_ON_NULL_RETURN_VAL(map_fb, EINA_FALSE);
        map_fb->locked = EINA_FALSE;
     }
   else map_fb = cmd->mask;

   map_start = (uint32_t *) _buffer_map_all(map_fb->buffer, &map_len, E_READ, E_ARGB, &map_stride);
   EINA_SAFETY_ON_FALSE_GOTO(src && dst && map_start, end);

   _filter_displace_cpu_alpha_do(w, h, map_w, map_h, intensity,
                                 src, dst, map_start, stretch, smooth, blend);

   ret = EINA_TRUE;
end:
   ector_buffer_unmap(cmd->input->buffer, src, src_len);
   ector_buffer_unmap(cmd->output->buffer, dst, dst_len);
   ector_buffer_unmap(map_fb->buffer, map_start, map_len);
   return ret;
}

/**
 * Apply distortion map on rgba image
 * input:  rgba
 * output: rgba
 * map:    rg+a (rgba)
 */
static Eina_Bool
_filter_displace_cpu_rgba(Evas_Filter_Command *cmd)
{
   unsigned int src_len, src_stride, map_len, map_stride, dst_len, dst_stride;
   int w, h, map_w, map_h, intensity;
   uint32_t *dst, *src, *map_start;
   Eina_Bool stretch, smooth, blend;
   Evas_Filter_Buffer *map_fb;
   Eina_Bool ret = EINA_FALSE;

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w == cmd->output->w, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(h == cmd->output->h, EINA_FALSE);

   src = _buffer_map_all(cmd->input->buffer, &src_len, E_READ, E_ARGB, &src_stride);
   dst = _buffer_map_all(cmd->output->buffer, &dst_len, E_READ | E_WRITE, E_ARGB, &dst_stride);
   stretch = cmd->displacement.flags & EVAS_FILTER_DISPLACE_STRETCH;
   smooth = cmd->displacement.flags & EVAS_FILTER_DISPLACE_LINEAR;
   map_w = cmd->mask->w;
   map_h = cmd->mask->h;
   intensity = cmd->displacement.intensity;
   blend = (cmd->draw.rop == EFL_GFX_RENDER_OP_BLEND);

   // Stretch if necessary.
   if ((map_w != w || map_h != h) && (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_XY))
     {
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
          map_w = w;
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          map_h = h;

        BUFFERS_LOCK();
        map_fb = cmd->ctx->buffer_scaled_get(cmd->ctx, cmd->mask, map_w, map_h);
        BUFFERS_UNLOCK();

        EINA_SAFETY_ON_NULL_RETURN_VAL(map_fb, EINA_FALSE);
        map_fb->locked = EINA_FALSE;
     }
   else map_fb = cmd->mask;

   map_start = _buffer_map_all(map_fb->buffer, &map_len, E_READ, E_ARGB, &map_stride);
   EINA_SAFETY_ON_FALSE_GOTO(src && dst && map_start, end);

   _filter_displace_cpu_rgba_do(w, h, map_w, map_h, intensity,
                                src, dst, map_start, stretch, smooth, blend);

   ret = EINA_TRUE;
end:
   ector_buffer_unmap(cmd->input->buffer, src, src_len);
   ector_buffer_unmap(cmd->output->buffer, dst, dst_len);
   ector_buffer_unmap(map_fb->buffer, map_start, map_len);
   return ret;
}

Software_Filter_Func
eng_filter_displace_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask, NULL);

   if (cmd->output->alpha_only)
     return _filter_displace_cpu_alpha;
   else
     return _filter_displace_cpu_rgba;
}
