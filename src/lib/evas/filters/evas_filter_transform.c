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
   size_t datasize, stride;
   DATA8 *in, *out, *span = NULL;
   int w, h, sy, dy, oy, center, t, b, objh;
   int s0, s1, d0, d1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input->backing, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output->backing, EINA_FALSE);

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->output->w == w, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->output->h == h, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->output->alpha_only == cmd->input->alpha_only, EINA_FALSE);

   in = ((RGBA_Image *) cmd->input->backing)->mask.data;
   out = ((RGBA_Image *) cmd->output->backing)->mask.data;
   datasize = cmd->input->alpha_only ? sizeof(DATA8) : sizeof(DATA32);
   stride = w * datasize;

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
        span = malloc(stride);
        if (!span) return EINA_FALSE;
     }

   for (sy = s0, dy = d0; (dy >= d1) && (sy <= s1); sy++, dy--)
     {
        DATA8* src = in + stride * sy;
        DATA8* dst = out + stride * dy;

        if (in == out)
          {
             if (src == dst) break;
             memcpy(span, dst, stride);
             memcpy(dst, src, stride);
             memcpy(src, span, stride);
             if (sy >= center) break;
          }
        else
          memcpy(dst, src, stride);
     }
   free(span);
   return EINA_TRUE;
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
