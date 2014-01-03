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
   int sw, sh, dx, dy, dw, dh, sx, sy;
   Eina_Bool repeat_x = EINA_FALSE, repeat_y = EINA_FALSE;

   if (!evas_filter_buffer_alloc(cmd->output, cmd->output->w, cmd->output->h))
     return EINA_FALSE;

   in = cmd->input->backing;
   out = cmd->output->backing;
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, EINA_FALSE);

   sx = 0;
   sy = 0;
   sw = in->cache_entry.w;
   sh = in->cache_entry.h;

   dx = cmd->draw.ox;
   dy = cmd->draw.oy;
   dw = MIN(out->cache_entry.w - dx, out->cache_entry.w);
   dh = MIN(out->cache_entry.h - dy, out->cache_entry.h);

   drawctx = cmd->ENFN->context_new(cmd->ENDT);
   cmd->ENFN->context_color_set(cmd->ENDT, drawctx, cmd->draw.R, cmd->draw.G,
                                cmd->draw.B, cmd->draw.A);
   cmd->ENFN->context_render_op_set(cmd->ENDT, drawctx, cmd->draw.render_op);

   if (cmd->draw.clip_use)
     cmd->ENFN->context_clip_set(cmd->ENDT, drawctx,
                                 cmd->draw.clip.x, cmd->draw.clip.y,
                                 cmd->draw.clip.w, cmd->draw.clip.h);
   else
     cmd->ENFN->context_clip_set(cmd->ENDT, drawctx, dx, dy, dw, dh);

   if (((dw != sw) || (dh != sh)) && cmd->draw.fillmode)
     {
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
          dw = out->cache_entry.w - dx;
        else if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_REPEAT_X)
          repeat_x = EINA_TRUE;
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          dh = out->cache_entry.h - dy;
        else if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_REPEAT_Y)
          repeat_y = EINA_TRUE;
     }
   else if (cmd->draw.fillmode == EVAS_FILTER_FILL_MODE_NONE)
     {
        int src_w = dw;
        int src_h = dh;
        _clip_to_target(&sx, &sy, sw, sh, dx, dy, out->cache_entry.w, out->cache_entry.h, &dx, &dy, &dh, &dw);
        if (src_w < sw) sw = src_w;
        if (src_h < sh) sh = src_h;
     }

   if (!repeat_x && !repeat_y)
     {
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
     }
   else
     {
        CRI("repeat not implemented yet");
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
