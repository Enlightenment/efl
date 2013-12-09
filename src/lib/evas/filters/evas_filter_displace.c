#include "evas_filter.h"
#include "evas_filter_private.h"

static void
_filter_displace_cpu_alpha_do(int w, int h, int map_w, int map_h, int map_step,
                              int dx, int dy, int intensity,
                              DATA8 *dst, DATA8 *src, DATA8 *map_start,
                              Eina_Bool displace_x, Eina_Bool displace_y,
                              Eina_Bool stretch, Eina_Bool smooth)
{
   int x, y, map_x, map_y;
   const int map_stride = map_w * map_step;
   DATA8 *map;

   for (y = 0, map_y = 0; y < h; y++, map_y++)
     {
        if (map_y >= map_h) map_y = 0;
        map = map_start + (map_y * map_stride);

        for (x = 0, map_x = 0; x < w; x++, dst++, src++, map_x++)
          {
             int offx = 0, offy = 0, offx_dec = 0, offy_dec = 0, val = 0;
             Eina_Bool out = 0;

             if (map_x >= map_w)
               {
                  map_x = 0;
                  map = map_start + (map_y * map_stride);
               }
             else map += map_step;

             if (displace_x)
               {
                  val = ((int) map[dx] - 128) * intensity;
                  offx = val >> 7;
                  offx_dec = val & 0x7f;
                  if ((x + offx) < 0) { offx = -x; out = 1; }
                  if ((x + offx + 1) >= w) { offx = w - x - 2; out = 1; }
               }
             if (displace_y)
               {
                  val = ((int) map[dy] - 128) * intensity;
                  offy = val >> 7;
                  offy_dec = val & 0x7f;
                  if ((y + offy) < 0) { offy = -y; out = 1; }
                  if ((y + offy + 1) >= h) { offy = h - y - 2; out = 1; }
               }

             if (out && !stretch)
               *dst = 0;
             else
               {
                  if (!smooth)
                    *dst = src[offx + offy * w];
                  else if (displace_x && displace_y)
                    {
                       val  = src[offx + offy * w] * (128 - offx_dec) * (128 - offy_dec);
                       val += src[offx + 1 + offy * w] * offx_dec * (128 - offy_dec);
                       val += src[offx + (offy + 1) * w] * (128 - offx_dec) * offy_dec;
                       val += src[offx + 1 + (offy + 1) * w] * offx_dec * offy_dec;
                       *dst = val >> 14; // <=> *dst = val / (128 * 128)
                    }
                  else
                    {
                       if (displace_x)
                         {
                            val  = (int) src[offx + offy * w] * (128 - offx_dec);
                            val += (int) src[offx + 1 + offy * w] * offx_dec;
                         }
                       else
                         {
                            val  = (int) src[offx + offy * w] * (128 - offy_dec);
                            val += (int) src[offx + (offy + 1) * w] * offy_dec;
                         }
                       *dst = val >> 7; // <=> *dst = val / 128
                    }
               }
          }
     }
}

static void
_filter_displace_cpu_rgba_do(int w, int h, int map_w, int map_h, int map_step,
                             int dx, int dy, int intensity,
                             DATA8 *map_start, DATA32 *src, DATA32 *dst,
                             Eina_Bool displace_x, Eina_Bool displace_y,
                             Eina_Bool stretch, Eina_Bool smooth)
{
   int x, y, map_x, map_y;
   const int map_stride = map_step * map_w;
   DATA8 *map;

   for (y = 0, map_y = 0; y < h; y++, map_y++)
     {
        if (map_y >= map_h) map_y = 0;
        map = map_start + (map_y * map_stride);

        for (x = 0, map_x = 0; x < w; x++, dst++, src++, map_x++)
          {
             int offx = 0, offy = 0, offx_dec = 0, offy_dec = 0, val = 0;
             Eina_Bool out = 0;

             if (map_x >= map_w)
               {
                  map_x = 0;
                  map = map_start + (map_y * map_stride);
               }
             else map += map_step;

             if (displace_x)
               {
                  val = ((int) map[dx] - 128) * intensity;
                  offx = val >> 7;
                  offx_dec = val & 0x7f;
                  if ((x + offx) < 0) { offx = -x; out = 1; }
                  if ((x + offx + 1) >= w) { offx = w - x - 2; out = 1; }
               }
             if (displace_y)
               {
                  val = ((int) map[dy] - 128) * intensity;
                  offy = val >> 7;
                  offy_dec = val & 0x7f;
                  if ((y + offy) < 0) { offy = -y; out = 1; }
                  if ((y + offy + 1) >= h) { offy = h - y - 2; out = 1; }
               }

             if (out && !stretch)
               *dst = A_VAL(src + offx + offy * w) << (ALPHA * 8);
             else if (!smooth)
               *dst = src[offx + offy * w];
             else if (displace_x && displace_y)
               {
                  int R, G, B, A;
                  DATA32 s00, s01, s10, s11; // indexes represent x,y
                  int mul00, mul01, mul10, mul11;

                  mul00 = (128 - offx_dec) * (128 * offy_dec);
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

                  R >>= 14;
                  G >>= 14;
                  B >>= 14;

                  *dst = ARGB_JOIN(A, R, G, B);
               }
             else if (displace_x)
               {
                  int R, G, B, A;
                  DATA32 s00, s10;
                  int mul00, mul10;

                  mul00 = (128 - offx_dec);
                  mul10 = offx_dec;

                  s00 = src[offx + offy * w];
                  s10 = src[offx + 1 + offy * w];

                  A = (ALPHA_OF(s00) * mul00) + (ALPHA_OF(s10) * mul10);
                  R = (RED_OF(s00) * mul00)   + (RED_OF(s10) * mul10);
                  G = (GREEN_OF(s00) * mul00) + (GREEN_OF(s10) * mul10);
                  B = (BLUE_OF(s00) * mul00)  + (BLUE_OF(s10) * mul10);

                  A >>= 7;
                  R >>= 7;
                  G >>= 7;
                  B >>= 7;

                  *dst = ARGB_JOIN(A, R, G, B);
               }
             else
               {
                  int R, G, B, A;
                  DATA32 s00, s01;
                  int mul00, mul01;

                  mul00 = (128 * offy_dec);
                  mul01 = offy_dec;

                  s00 = src[offx + offy * w];
                  s01 = src[offx + (offy + 1)* w];

                  A = (ALPHA_OF(s00) * mul00) + (ALPHA_OF(s01) * mul01);
                  R = (RED_OF(s00) * mul00)   + (RED_OF(s01) * mul01);
                  G = (GREEN_OF(s00) * mul00) + (GREEN_OF(s01) * mul01);
                  B = (BLUE_OF(s00) * mul00)  + (BLUE_OF(s01) * mul01);

                  A >>= 7;
                  R >>= 7;
                  G >>= 7;
                  B >>= 7;

                  *dst = ARGB_JOIN(A, R, G, B);
               }
          }
     }
}

/**
 * Apply distortion map on alpha image
 * input:  alpha
 * output: alpha
 * map:    alpha or rgba
 * direction: X, Y or XY
 */
static Eina_Bool
_filter_displace_cpu_alpha(Evas_Filter_Command *cmd)
{
   int w, h, map_w, map_h, intensity, map_step, dx = 0, dy = 0;
   DATA8 *dst, *src, *map_start;
   Eina_Bool displace_x, displace_y, stretch, smooth;

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w == cmd->output->w, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(h == cmd->output->h, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input->backing, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask->backing, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output->backing, EINA_FALSE);

   src = ((RGBA_Image *) cmd->input->backing)->mask.data;
   map_start = ((RGBA_Image *) cmd->mask->backing)->mask.data;
   dst = ((RGBA_Image *) cmd->output->backing)->mask.data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(map_start, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);

   displace_x = cmd->displacement.flags & EVAS_FILTER_DISPLACE_X;
   displace_y = cmd->displacement.flags & EVAS_FILTER_DISPLACE_Y;
   stretch = cmd->displacement.flags & EVAS_FILTER_DISPLACE_STRETCH;
   smooth = cmd->displacement.flags & EVAS_FILTER_DISPLACE_LINEAR;
   map_w = cmd->mask->w;
   map_h = cmd->mask->h;
   intensity = cmd->displacement.intensity;

   if (cmd->mask->alpha_only)
     map_step = sizeof(DATA8);
   else
     {
        map_step = sizeof(DATA32);
        if (cmd->displacement.flags & EVAS_FILTER_DISPLACE_RG)
          {
             dx = RED;
             dy = GREEN;
          }
        else dx = dy = ALPHA;
     }

   _filter_displace_cpu_alpha_do(w, h, map_w, map_h, map_step, dx, dy,
                                 intensity, dst, src, map_start,
                                 displace_x, displace_y, stretch, smooth);

   return EINA_TRUE;
}

/**
 * Apply distortion map on rgba image
 * input:  rgba
 * output: rgba
 * map:    alpha or rgba
 * direction: X, Y or XY
 */
static Eina_Bool
_filter_displace_cpu_rgba(Evas_Filter_Command *cmd)
{
   int w, h, map_w, map_h, intensity, map_step, dx, dy;
   DATA32 *dst, *src;
   DATA8 *map_start;
   Eina_Bool displace_x, displace_y, stretch, smooth;

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w == cmd->output->w, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(h == cmd->output->h, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input->backing, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask->backing, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output->backing, EINA_FALSE);

   src = ((RGBA_Image *) cmd->input->backing)->image.data;
   map_start = ((RGBA_Image *) cmd->mask->backing)->mask.data;
   dst = ((RGBA_Image *) cmd->output->backing)->image.data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(map_start, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);

   displace_x = cmd->displacement.flags & EVAS_FILTER_DISPLACE_X;
   displace_y = cmd->displacement.flags & EVAS_FILTER_DISPLACE_Y;
   stretch = cmd->displacement.flags & EVAS_FILTER_DISPLACE_STRETCH;
   smooth = cmd->displacement.flags & EVAS_FILTER_DISPLACE_LINEAR;
   map_w = cmd->mask->w;
   map_h = cmd->mask->h;
   intensity = cmd->displacement.intensity;

   if (!displace_x && !displace_y)
     {
        WRN("Invalid displacement flags! Defaulting to XY displacement.");
        displace_x = displace_y = EINA_TRUE;
     }

   if (cmd->mask->alpha_only)
     {
        map_step = sizeof(DATA8);
        dx = dy = 0;
     }
   else
     {
        map_step = sizeof(DATA32);
        if (cmd->displacement.flags & EVAS_FILTER_DISPLACE_RG)
          {
             dx = RED;
             dy = GREEN;
          }
        else dx = dy = ALPHA;
     }

   _filter_displace_cpu_rgba_do(w, h, map_w, map_h, map_step, dx, dy,
                                intensity, map_start, src, dst,
                                displace_x, displace_y, stretch, smooth);

   return EINA_TRUE;
}

Evas_Filter_Apply_Func
evas_filter_displace_cpu_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask, NULL);

   if (cmd->input->alpha_only != cmd->output->alpha_only)
     {
        CRIT("Invalid color formats");
        return NULL;
     }

   if (cmd->input->alpha_only)
     {
        if ((cmd->displacement.flags & EVAS_FILTER_DISPLACE_RG)
            && cmd->mask->alpha_only)
          {
             goto invalid_flags;
          }
        return _filter_displace_cpu_alpha;
     }
   else
     {
        if ((cmd->displacement.flags & EVAS_FILTER_DISPLACE_RG)
            && cmd->mask->alpha_only)
          {
             goto invalid_flags;
          }
        return _filter_displace_cpu_rgba;
     }

invalid_flags:
   ERR("Incompatible flags (0x%02x) and data (input %s, output %s, map %s)",
       (cmd->displacement.flags & EVAS_FILTER_DISPLACE_BITMASK),
       cmd->input->alpha_only ? "alpha" : "rgba",
       cmd->output->alpha_only ? "alpha" : "rgba",
       cmd->mask->alpha_only ? "alpha" : "rgba");
   return NULL;
}
