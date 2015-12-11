#include "evas_filter_private.h"

/* Apply geomeetrical transformations to a buffer.
 * This filter is a bit simplistic at the moment.
 *
 * It also assumes the destination is empty, as it does not use blend
 * operations. This should probably be fixed later on (use evas_map?).
 */

static Eina_Bool
_vflip_cpu(Evas_Filter_Command *cmd)
{
   unsigned int src_len, src_stride, dst_len, dst_stride;
   uint8_t *in, *out = NULL, *span = NULL;
   int w, h, sy, dy, oy, center, t, b, objh;
   Efl_Gfx_Colorspace cspace = cmd->output->alpha_only ? E_ALPHA : E_ARGB;
   int s0, s1, d0, d1;
   Eina_Bool ret = 0;

   w = cmd->input->w;
   h = cmd->input->h;
   in = _buffer_map_all(cmd->input->buffer, &src_len, E_READ, cspace, &src_stride);
   if (cmd->input->buffer != cmd->output->buffer)
     out = _buffer_map_all(cmd->output->buffer, &dst_len, E_WRITE, cspace, &dst_stride);

   EINA_SAFETY_ON_FALSE_GOTO(cmd->output->w == w, end);
   EINA_SAFETY_ON_FALSE_GOTO(cmd->output->h == h, end);
   EINA_SAFETY_ON_FALSE_GOTO(src_stride <= dst_stride, end);

   oy = cmd->draw.oy;
   t = cmd->ctx->padt;
   b = cmd->ctx->padb;
   objh = h - t - b;
   center = t + objh / 2 + oy;

   s0 = t;
   s1 = h - b - 1;
   if (oy >= 0)
     {
        d0 = center + (objh / 2) + oy;
        d1 = center - (objh / 2) - oy;
     }
   else
     {
        d0 = center + (objh / 2) - oy;
        d1 = center - (objh / 2) + oy;
     }

   if (in == out)
     {
        span = alloca(src_stride);
        if (!span) goto end;
     }

   for (sy = s0, dy = d0; (dy >= d1) && (sy <= s1); sy++, dy--)
     {
        uint8_t* src = in + src_stride * sy;
        uint8_t* dst = out + dst_stride * dy;

        if (in == out)
          {
             if (src == dst) break;
             memcpy(span, dst, src_stride);
             memcpy(dst, src, src_stride);
             memcpy(src, span, src_stride);
             if (sy >= center) break;
          }
        else
          memcpy(dst, src, src_stride);
     }
   ret = EINA_TRUE;

end:
   eo_do(cmd->input->buffer, ector_buffer_unmap(in, src_len));
   if (in != out) eo_do(cmd->output->buffer, ector_buffer_unmap(out, dst_len));
   return ret;
}

Evas_Filter_Apply_Func
evas_filter_transform_cpu_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);

   switch (cmd->transform.flags)
     {
      case EVAS_FILTER_TRANSFORM_VFLIP:
        return _vflip_cpu;
      default:
        CRI("Unknown transform flag %d", (int) cmd->transform.flags);
        return NULL;
     }
}
