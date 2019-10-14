#include "evas_engine_filter.h"

static Eina_Bool
_evas_filter_grayscale(Evas_Filter_Command *cmd)
{
   int sw, sh, dw, dh, x, y, slen, dlen;
   unsigned int src_len, src_stride, dst_len, dst_stride;
   Eina_Bool ret = EINA_FALSE;
   DATA32 *ts, *td, *src = NULL, *dst = NULL;
   DATA8 r, g, b, gry;

   ector_buffer_size_get(cmd->input->buffer, &sw, &sh);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((sw > 0) && (sh > 0), ret);

   ector_buffer_size_get(cmd->output->buffer, &dw, &dh);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((dw > 0) && (dh > 0), ret);

   src = _buffer_map_all(cmd->input->buffer, &src_len, E_READ, E_ARGB, &src_stride);
   EINA_SAFETY_ON_FALSE_GOTO(src, end);

   dst = _buffer_map_all(cmd->output->buffer, &dst_len, E_WRITE, E_ARGB, &dst_stride);
   EINA_SAFETY_ON_FALSE_GOTO(dst, end);

   slen = src_stride / sizeof(*src);
   dlen = dst_stride / sizeof(*dst);

   ts = src;
   td = dst;
   for (y = 0; y < sh ; y++)
     {
        for (x = 0; x < sw; x++)
          {
             A_VAL(td + x) = A_VAL(ts + x);
             r = R_VAL(ts + x);
             g = G_VAL(ts + x);
             b = B_VAL(ts + x);

             /* formula from evas_common_convert_rgba_to_8bpp_gry_256_dith */
             gry = ((r * 19596) + (g * 38470) + (b * 7472)) >> 16;
             R_VAL(td + x) = G_VAL(td + x) = B_VAL(td + x) = gry;
          }
          ts += slen;
          td += dlen;
     }

   ret = EINA_TRUE;

end:
   if (src) ector_buffer_unmap(cmd->input->buffer, src, src_len);
   if (dst) ector_buffer_unmap(cmd->output->buffer, dst, dst_len);
   return ret;
}

Software_Filter_Func
eng_filter_grayscale_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   return _evas_filter_grayscale;
}
