#include "evas_filter.h"
#include "evas_filter_private.h"
#include "evas_blend_private.h"

#if DIV_USING_BITSHIFT
static int
_smallest_pow2_larger_than(int val)
{
   int n;

   for (n = 0; n < 32; n++)
     if (val <= (1 << n)) return n;

   ERR("Value %d is too damn high!", val);
   return 32;
}
# define DEFINE_DIVIDER(div) const int pow2 = _smallest_pow2_larger_than((div) << 10); const int numerator = (1 << pow2) / (div);
# define DIVIDE(val) (((val) * numerator) >> pow2)
#else
# define DEFINE_DIAMETER(div) const int divider = (div);
# define DIVIDE(val) ((val) / divider)
#endif

static Eina_Bool
_filter_blend_cpu_alpha(Evas_Filter_Command *cmd)
{
   RGBA_Image *in, *out;
   Alpha_Gfx_Func func;
   DATA8 *maskdata, *dstdata;
   int sw, sh, dw, dh, ox, oy, sx = 0, sy = 0, dx = 0, dy = 0, rows, cols, y;

   func = evas_common_alpha_func_get(cmd->draw.render_op);
   if (!func)
     return EINA_FALSE;

   if (!evas_filter_buffer_alloc(cmd->output, cmd->output->w, cmd->output->h))
     return EINA_FALSE;

   in = cmd->input->backing;
   out = cmd->output->backing;
   sw = in->cache_entry.w;
   sh = in->cache_entry.h;
   dw = out->cache_entry.w;
   dh = out->cache_entry.h;
   ox = cmd->draw.ox;
   oy = cmd->draw.oy;
   maskdata = in->mask.data;
   dstdata = out->mask.data;

   if (!ox && !oy && (dw == sw) && (dh == sh))
     {
        func(maskdata, dstdata, sw * sh);
        return EINA_TRUE;
     }

   _clip_to_target(&sx, &sy, sw, sh, ox, oy, dw, dh, &dx, &dy, &rows, &cols);
   // FIXME/TODO: Clip to context clip

   if (cols <= 0 || rows <= 0)
     return EINA_TRUE;

   maskdata += sy * sw;
   dstdata += dy * dw;
   for (y = rows; y; y--)
     {
        func(maskdata + sx, dstdata + dx, cols);
        maskdata += sw;
        dstdata += dw;
     }

   return EINA_TRUE;
}

static Eina_Bool
_filter_blend_cpu_rgba(Evas_Filter_Command *cmd)
{
   RGBA_Image *in, *out;
   RGBA_Draw_Context *drawctx;
   int sw, sh, dx, dy, dw, dh, sx, sy, ox, oy;
   int row, col, rows = 1, cols = 1;
   int right = 0, bottom = 0, left = 0, top = 0;

   in = cmd->input->backing;
   out = cmd->output->backing;
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, EINA_FALSE);

   sx = 0;
   sy = 0;
   sw = in->cache_entry.w;
   sh = in->cache_entry.h;

   ox = dx = cmd->draw.ox;
   oy = dy = cmd->draw.oy;
   dw = MIN(out->cache_entry.w - dx, out->cache_entry.w);
   dh = MIN(out->cache_entry.h - dy, out->cache_entry.h);

   if ((dw <= 0) || (dh <= 0) || (sw <= 0) || (sh <= 0))
     return EINA_TRUE;

   if (!evas_filter_buffer_alloc(cmd->output, cmd->output->w, cmd->output->h))
     return EINA_FALSE;

   drawctx = cmd->ENFN->context_new(cmd->ENDT);
   cmd->ENFN->context_color_set(cmd->ENDT, drawctx, cmd->draw.R, cmd->draw.G,
                                cmd->draw.B, cmd->draw.A);
   cmd->ENFN->context_render_op_set(cmd->ENDT, drawctx, cmd->draw.render_op);

   if (cmd->draw.clip_use)
     {
        cmd->ENFN->context_clip_set(cmd->ENDT, drawctx,
                                    cmd->draw.clip.x, cmd->draw.clip.y,
                                    cmd->draw.clip.w, cmd->draw.clip.h);
        cmd->ENFN->context_clip_clip(cmd->ENDT, drawctx,0, 0,
                                     out->cache_entry.w, out->cache_entry.h);
     }
   else
     {
        cmd->ENFN->context_clip_set(cmd->ENDT, drawctx, 0, 0,
                                    out->cache_entry.w, out->cache_entry.h);
     }

   if (cmd->draw.fillmode == EVAS_FILTER_FILL_MODE_NONE)
     {
        int src_w = dw;
        int src_h = dh;
        _clip_to_target(&sx, &sy, sw, sh, dx, dy, out->cache_entry.w,
                        out->cache_entry.h, &dx, &dy, &dh, &dw);
        if (src_w < sw) sw = src_w;
        if (src_h < sh) sh = src_h;

        DBG("blend: %d,%d,%d,%d --> %d,%d,%d,%d (from %dx%d to %dx%d +%d,%d)",
            0, 0, sw, sh, dx, dy, dw, dh,
            in->cache_entry.w, in->cache_entry.h,
            out->cache_entry.w, out->cache_entry.h,
            dx, dy);
        cmd->ENFN->image_draw(cmd->ENDT, drawctx, out, in,
                              sx, sy, sw, sh, // src
                              dx, dy, dw, dh, // dst
                              EINA_TRUE, // smooth
                              EINA_FALSE); // Not async
        return EINA_TRUE;
     }

   if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_REPEAT_X)
     {
        if (ox > 0) left = ox % sw;
        else if (ox < 0) left = sw + (ox % sw);
        cols = (dw  - left) / sw;
        right = dw - (sw * cols) - left;
        dx = 0;
     }
   else if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
     {
        cols = 1;
        dw = out->cache_entry.w;
        dx = 0;
     }
   else
     {
        cols = 1;
        dw = out->cache_entry.w - ox;
        dx = ox;
     }

   if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_REPEAT_Y)
     {
        if (oy > 0) top = oy % sh;
        else if (oy < 0) top = sh + (oy % sh);
        rows = (dh - top) / sh;
        bottom = dh - (sh * rows) - top;
        dy = 0;
     }
   else if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
     {
        rows = 1;
        dh = out->cache_entry.h;
        dy = 0;
     }
   else
     {
        rows = 1;
        dh = out->cache_entry.h - oy;
        dy = oy;
     }

   if (top > 0) row = -1;
   else row = 0;
   for (; row <= rows; row++)
     {
        int src_x, src_y, src_w, src_h;
        int dst_x, dst_y, dst_w, dst_h;

        if (row == -1 && top > 0)
          {
             src_h = top;
             src_y = sh - top;
             dst_y = dy;
          }
        else if (row == rows)
          {
             src_h = bottom;
             src_y = 0;
             dst_y = top + dy + row * sh;
          }
        else
          {
             src_h = sh;
             src_y = 0;
             dst_y = top + dy + row * sh;
          }
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          dst_h = dh;
        else
           dst_h = MIN(dh - dst_y, src_h);
        if (src_h <= 0 || dst_h <= 0) break;

        if (left > 0) col = -1;
        else col = 0;
        for (; col <= cols; col++)
          {
             if (col == -1 && left > 0)
               {
                  src_w = left;
                  src_x = sw - left;
                  dst_x = dx;
               }
             else if (col == cols)
               {
                  src_w = right;
                  src_x = 0;
                  dst_x = left + dx + col * sw;
               }
             else
               {
                  src_w = sw;
                  src_x = 0;
                  dst_x = left + dx + col * sw;
               }
             if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
               dst_w = dw;
             else
                dst_w = MIN(dw - dst_x, src_w);
             if (src_w <= 0 || dst_w <= 0) break;

             DBG("blend: [%d,%d] %d,%d,%dx%d --> %d,%d,%dx%d "
                 "(src %dx%d, dst %dx%d, offset %d,%d)",
                 col, row, src_x, src_y, src_w, src_h,
                 dst_x, dst_y, dst_w, dst_h,
                 sw, sh, dw, dh, ox, oy);
             cmd->ENFN->image_draw(cmd->ENDT, drawctx, out, in,
                                   src_x, src_y, src_w, src_h,
                                   dst_x, dst_y, dst_w, dst_h,
                                   EINA_TRUE, EINA_FALSE);
          }
     }

   cmd->ENFN->context_free(cmd->ENDT, drawctx);

   return EINA_TRUE;
}

static Eina_Bool
_filter_blend_cpu_mask_rgba(Evas_Filter_Command *cmd)
{
   RGBA_Image *in, *out;
   RGBA_Gfx_Func func;
   DATA32 col;
   DATA32 *dstdata;
   DATA8 *maskdata;
   int sw, sh, dw, dh, ox, oy, sx = 0, sy = 0, dx = 0, dy = 0, rows, cols, y;

   if (!evas_filter_buffer_alloc(cmd->output, cmd->output->w, cmd->output->h))
     return EINA_FALSE;

   in = cmd->input->backing;
   out = cmd->output->backing;
   sw = in->cache_entry.w;
   sh = in->cache_entry.h;
   dw = out->cache_entry.w;
   dh = out->cache_entry.h;
   ox = cmd->draw.ox;
   oy = cmd->draw.oy;
   dstdata = out->image.data;
   maskdata = in->mask.data;
   col = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);

   func = evas_common_gfx_func_composite_mask_color_span_get
     (col, out, 1, cmd->draw.render_op);

   if (!func)
     return EINA_FALSE;

   if (!ox && !oy && (dw == sw) && (dh == sh))
     {
        func(NULL, maskdata, col, dstdata, sw * sh);
        return EINA_TRUE;
     }

   _clip_to_target(&sx, &sy, sw, sh, ox, oy, dw, dh, &dx, &dy, &rows, &cols);
   // FIXME/TODO: Clip to context clip

   if (cols <= 0 || rows <= 0)
     return EINA_TRUE;

   maskdata += sy * sw;
   dstdata += dy * dw;
   for (y = rows; y; y--)
     {
        func(NULL, maskdata + sx, col, dstdata + dx, cols);
        maskdata += sw;
        dstdata += dw;
     }

   return EINA_TRUE;
}

static Eina_Bool
_filter_blend_cpu_rgba2alpha(Evas_Filter_Command *cmd)
{
   RGBA_Image *in, *out;
   DATA8 *dstdata;
   DATA32 *srcdata;
   int sw, sh, dw, dh, ox, oy, sx = 0, sy = 0, dx = 0, dy = 0, rows, cols, y, x;
   DEFINE_DIVIDER(3);

   if (!evas_filter_buffer_alloc(cmd->output, cmd->output->w, cmd->output->h))
     return EINA_FALSE;

   in = cmd->input->backing;
   out = cmd->output->backing;
   sw = in->cache_entry.w;
   sh = in->cache_entry.h;
   dw = out->cache_entry.w;
   dh = out->cache_entry.h;
   ox = cmd->draw.ox;
   oy = cmd->draw.oy;
   srcdata = in->image.data;
   dstdata = out->mask.data;

   _clip_to_target(&sx, &sy, sw, sh, ox, oy, dw, dh, &dx, &dy, &rows, &cols);
   // FIXME/TODO: Clip to context clip

   if (cols <= 0 || rows <= 0)
     return EINA_TRUE;

   srcdata += sy * sw;
   dstdata += dy * dw;
   for (y = rows; y; y--)
     {
        DATA32 *s = srcdata + sx;
        DATA8 *d = dstdata + dx;
        for (x = cols; x; x--, d++, s++)
          {
             // TODO: Add weights?
             *d = DIVIDE(R_VAL(s) + G_VAL(s) + B_VAL(s));
          }
        srcdata += sw;
        dstdata += dw;
     }

   return EINA_TRUE;
}

Evas_Filter_Apply_Func
evas_filter_blend_cpu_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   if (cmd->input->alpha_only)
     {
        if (cmd->output->alpha_only)
          return _filter_blend_cpu_alpha;
        else
          return _filter_blend_cpu_mask_rgba;
     }
   else
     {
        if (cmd->output->alpha_only)
          return _filter_blend_cpu_rgba2alpha;
        else
          return _filter_blend_cpu_rgba;
     }
}
