#include "evas_filter.h"
#include "evas_filter_private.h"


static Eina_Bool
_filter_curve_cpu_rgba(Evas_Filter_Command *cmd)
{
   RGBA_Image *in, *out;
   DATA32 *src, *dst, *d, *s;
   DATA8 *curve;
   int k, offset = -1, len;

#define C_VAL(p) (((DATA8 *)(p))[offset])

   in = cmd->input->backing;
   out = cmd->output->backing;
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, EINA_FALSE);
   src = in->image.data;
   dst = out->image.data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);
   curve = cmd->curve.data;
   len = in->cache_entry.w * in->cache_entry.h;

   switch (cmd->curve.channel)
     {
#ifndef WORDS_BIGENDIAN
      case EVAS_FILTER_CHANNEL_RED:   offset = 2; break;
      case EVAS_FILTER_CHANNEL_GREEN: offset = 1; break;
      case EVAS_FILTER_CHANNEL_BLUE:  offset = 0; break;
#else
      case EVAS_FILTER_CHANNEL_RED:   offset = 1; break;
      case EVAS_FILTER_CHANNEL_GREEN: offset = 2; break;
      case EVAS_FILTER_CHANNEL_BLUE:  offset = 3; break;
#endif
      case EVAS_FILTER_CHANNEL_ALPHA: break;
      case EVAS_FILTER_CHANNEL_RGB: break;
      default:
        ERR("Invalid color channel %d", (int) cmd->curve.channel);
        return EINA_FALSE;
     }

   if (src != dst)
     memcpy(dst, src, len * sizeof(DATA32));
   evas_data_argb_unpremul(dst, len);

   // One channel (R, G or B)
   if (offset >= 0)
     {
        for (k = len, s = src, d = dst; k; k--, d++, s++)
          C_VAL(d) = curve[C_VAL(s)];

        goto premul;
     }

   // All RGB channels
   if (cmd->curve.channel == EVAS_FILTER_CHANNEL_RGB)
     {
#ifndef WORDS_BIGENDIAN
        for (offset = 0; offset <= 2; offset++)
#else
        for (offset = 1; offset <= 3; offset++)
#endif
          {
             for (k = len, s = src, d = dst; k; k--, d++, s++)
               C_VAL(d) = curve[C_VAL(s)];
          }

        goto premul;
     }

   // Alpha
#ifndef WORDS_BIGENDIAN
   offset = 3;
#else
   offset = 0;
#endif

   for (k = len, d = dst; k; k--, d++, src++)
     C_VAL(d) = curve[C_VAL(src)];

premul:
   evas_data_argb_premul(dst, len);
   return EINA_TRUE;
}

static Eina_Bool
_filter_curve_cpu_alpha(Evas_Filter_Command *cmd)
{
   RGBA_Image *in, *out;
   DATA8 *src, *dst;
   DATA8 *curve;
   int k;

   in = cmd->input->backing;
   out = cmd->output->backing;
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, EINA_FALSE);
   src = in->mask.data;
   dst = out->mask.data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);
   curve = cmd->curve.data;

   for (k = in->cache_entry.w * in->cache_entry.h; k; k--)
     *dst++ = curve[*src++];

   return EINA_TRUE;
}

Evas_Filter_Apply_Func
evas_filter_curve_cpu_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((cmd->input->w == cmd->output->w)
                                   && (cmd->input->h == cmd->output->h), 0);

   if (!cmd->input->alpha_only && !cmd->output->alpha_only)
     return _filter_curve_cpu_rgba;

   if (cmd->input->alpha_only && cmd->output->alpha_only)
     return _filter_curve_cpu_alpha;

   CRI("Incompatible image formats");
   return NULL;
}
