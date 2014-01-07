#include "evas_filter_private.h"

static Eina_Bool
_vflip_cpu(Evas_Filter_Command *cmd)
{
   size_t datasize, stride;
   DATA8 *in, *out, *span;
   int w, h, sy, dy;

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

   if (in == out)
     {
        span = malloc(stride);
        if (!span) return EINA_FALSE;
     }

   for (sy = 0, dy = h - 1; dy >= 0; sy++, dy--)
     {
        DATA8* src = in + stride * sy;
        DATA8* dst = out + stride * dy;

        if (in == out)
          {
             memcpy(span, dst, stride);
             memcpy(dst, src, stride);
             memcpy(src, span, stride);
             if (sy >= (h / 2)) break;
          }
        else
          memcpy(dst, src, stride);
     }

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
