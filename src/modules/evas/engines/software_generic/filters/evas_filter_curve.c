#include "evas_engine_filter.h"

static Eina_Bool
_filter_curve_cpu_rgba(Evas_Filter_Command *cmd)
{
   unsigned int src_len, src_stride, dst_len, dst_stride;
   void *src_map = NULL, *dst_map;
   Eina_Bool ret = EINA_FALSE;
   uint32_t *src, *dst, *d, *s;
   uint8_t *curve;
   int k, offset = -1, len;

#define C_VAL(p) (((uint8_t *)(p))[offset])

   // FIXME: support src_stride != dst_stride
   // Note: potentially mapping the same region twice (read then write)
   src_map = src = _buffer_map_all(cmd->input->buffer, &src_len, E_READ, E_ARGB, &src_stride);
   dst_map = dst = _buffer_map_all(cmd->output->buffer, &dst_len, E_WRITE, E_ARGB, &dst_stride);
   EINA_SAFETY_ON_FALSE_GOTO(src && dst && (src_len == dst_len), end);

   curve = cmd->curve.data;
   len = dst_len / sizeof(uint32_t);

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
        goto end;
     }

   if (src != dst)
     memcpy(dst, src, dst_len);
   efl_draw_argb_unpremul(dst, len);

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
   efl_draw_argb_premul(dst, len);
   ret = EINA_TRUE;

end:
   ector_buffer_unmap(cmd->input->buffer, src_map, src_len);
   ector_buffer_unmap(cmd->output->buffer, dst_map, dst_len);
   return ret;
}

static Eina_Bool
_filter_curve_cpu_alpha(Evas_Filter_Command *cmd)
{
   unsigned int src_len, src_stride, dst_len, dst_stride;
   uint8_t *src, *dst, *curve;
   void *src_map, *dst_map;
   Eina_Bool ret = EINA_FALSE;
   int k;

   // FIXME: support src_stride != dst_stride
   // Note: potentially mapping the same region twice (read then write)
   src_map = src = _buffer_map_all(cmd->input->buffer, &src_len, E_READ, E_ALPHA, &src_stride);
   dst_map = dst = _buffer_map_all(cmd->output->buffer, &dst_len, E_WRITE, E_ALPHA, &dst_stride);
   EINA_SAFETY_ON_FALSE_GOTO(src && dst && (src_len == dst_len), end);
   curve = cmd->curve.data;

   for (k = src_len; k; k--)
     *dst++ = curve[*src++];

   ret = EINA_TRUE;

end:
   ector_buffer_unmap(cmd->input->buffer, src_map, src_len);
   ector_buffer_unmap(cmd->output->buffer, dst_map, dst_len);
   return ret;
}

Software_Filter_Func
eng_filter_curve_func_get(Evas_Filter_Command *cmd)
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

   // Rely on ector buffer's implicit conversion. not great but the command
   // doesn't make much sense (curve requires same channel count).
   WRN("Incompatible image formats");
   return _filter_curve_cpu_rgba;
}
