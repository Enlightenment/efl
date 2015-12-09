#include "evas_filter_private.h"

static Eina_Bool
_fill_cpu(Evas_Filter_Command *cmd)
{
   Evas_Filter_Buffer *fb = cmd->output;
   int step = fb->alpha_only ? sizeof(DATA8) : sizeof(DATA32);
   int x = MAX(0, cmd->draw.clip.x);
   int y = MAX(0, cmd->draw.clip.y);
   DATA8 *ptr = ((RGBA_Image *) fb->backing)->image.data8;
   int w, h, k, j;

   if (!cmd->draw.clip_mode_lrtb)
     {
        if (cmd->draw.clip.w)
          w = MIN(cmd->draw.clip.w, fb->w - x);
        else
          w = fb->w - x;
        if (cmd->draw.clip.h)
          h = MIN(cmd->draw.clip.h, fb->h - y);
        else
          h = fb->h - y;
     }
   else
     {
        x = MAX(0, cmd->draw.clip.l);
        y = MAX(0, cmd->draw.clip.t);
        w = CLAMP(0, fb->w - x - cmd->draw.clip.r, fb->w - x);
        h = CLAMP(0, fb->h - y - cmd->draw.clip.b, fb->h - y);
     }

   ptr += y * step * fb->w;
   if ((fb->alpha_only)
       || (!cmd->draw.R && !cmd->draw.G && !cmd->draw.B && !cmd->draw.A)
       || ((cmd->draw.R == 0xff) && (cmd->draw.G == 0xff)
           && (cmd->draw.B == 0xff) && (cmd->draw.A == 0xff)))
     {
        for (k = 0; k < h; k++)
          {
             memset(ptr + (x * step), cmd->draw.A, step * w);
             ptr += step * fb->w;
          }
     }
   else
     {
        DATA32 *dst = ((DATA32 *) ptr) + x;
        DATA32 color = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);
        for (k = 0; k < h; k++)
          {
             for (j = 0; j < w; j++)
               *dst++ = color;
             dst += fb->w - w;
          }
     }

   return EINA_TRUE;
}

Evas_Filter_Apply_Func
evas_filter_fill_cpu_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   return _fill_cpu;
}
