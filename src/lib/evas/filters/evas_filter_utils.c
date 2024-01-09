/* Utility functions for the filters.  */

#include "evas_filter_private.h"

Evas_Filter_Buffer *
evas_filter_buffer_scaled_get(Evas_Filter_Context *ctx,
                              Evas_Filter_Buffer *src,
                              unsigned w, unsigned h)
{
   unsigned int src_len = 0, src_stride, dst_len = 0, dst_stride;
   uint8_t *src_map = NULL, *dst_map = NULL;
   Evas_Filter_Buffer *dst;
   RGBA_Image dstim, srcim;
   RGBA_Draw_Context dc;
   Eina_Bool ok;

   DBG("Scaling buffer from %dx%d to %dx%d, this is a slow operation!",
       src->w, src->h, w, h);
   DEBUG_TIME_BEGIN();

   // Get destination buffer
   dst = evas_filter_temporary_buffer_get(ctx, w, h, src->alpha_only, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, NULL);

   // Map input and output
   src_map = _buffer_map_all(src->buffer, &src_len, E_READ, E_ARGB, &src_stride);
   dst_map = _buffer_map_all(dst->buffer, &dst_len, E_WRITE, E_ARGB, &dst_stride);
   EINA_SAFETY_ON_FALSE_GOTO(src_map && dst_map, end);
   EINA_SAFETY_ON_FALSE_GOTO((src_stride == ((unsigned) src->w * 4)) &&
                             (dst_stride == (w * 4)), end);

   // Wrap as basic RGBA_Images
   memset(&dstim, 0, sizeof(dstim));
   dstim.cache_entry.w = w;
   dstim.cache_entry.h = h;
   dstim.cache_entry.flags.alpha = 1;
   dstim.cache_entry.flags.alpha_sparse = 0;
   dstim.cache_entry.space = EVAS_COLORSPACE_ARGB8888;
   dstim.image.data8 = dst_map;

   memset(&srcim, 0, sizeof(srcim));
   srcim.cache_entry.w = src->w;
   srcim.cache_entry.h = src->h;
   srcim.cache_entry.flags.alpha = 1;
   srcim.cache_entry.flags.alpha_sparse = 0;
   srcim.cache_entry.space = EVAS_COLORSPACE_ARGB8888;
   srcim.image.data8 = src_map;

   // Basic draw context
   memset(&dc, 0, sizeof(dc));
   dc.render_op = EVAS_RENDER_COPY;

   // Do the scale
   ok = evas_common_scale_rgba_in_to_out_clip_smooth
         (&srcim, &dstim, &dc, 0, 0, src->w, src->h, 0, 0, w, h);

   if (!ok)
     {
       ERR("RGBA Image scaling failed.");
       dst = NULL;
     }

end:
   if (src_map) ector_buffer_unmap(src->buffer, src_map, src_len);
   if ((dst) && (dst_map))
     ector_buffer_unmap(dst->buffer, dst_map, dst_len);
   DEBUG_TIME_END();
   return dst;
}

static Eina_Bool
_interpolate_none(DATA8 *output, int *points)
{
   DATA8 val = 0;
   int j;
   for (j = 0; j < 256; j++)
     {
        if (points[j] == -1)
          output[j] = val;
        else
          val = output[j] = (DATA8) points[j];
     }
   return EINA_TRUE;
}

static Eina_Bool
_interpolate_linear(DATA8 *output, int *points)
{
   DATA8 val = 0;
   int j, k, last_idx = 0;
   for (j = 0; j < 256; j++)
     {
        if (points[j] != -1)
          {
             int val1, val2, vald, pos, len;

             output[j] = (DATA8) points[j];
             val1 = points[last_idx];
             val2 = points[j];
             vald = val2 - val1;
             len = j - last_idx;
             for (k = last_idx + 1; k < j; k++)
               {
                  pos = k - last_idx;
                  output[k] = (DATA8) (val1 + ((pos * vald) / len));
               }
             last_idx = j;
          }
     }
   val = (DATA8) points[last_idx];
   for (j = last_idx + 1; j < 256; j++)
     output[j] = val;
   return EINA_TRUE;
}

Eina_Bool
evas_filter_interpolate(DATA8 *output, int *points,
                        Evas_Filter_Interpolation_Mode mode)
{
   switch (mode)
     {
      case EVAS_FILTER_INTERPOLATION_MODE_NONE:
        return _interpolate_none(output, points);
      case EVAS_FILTER_INTERPOLATION_MODE_LINEAR:
      default:
        return _interpolate_linear(output, points);
     }
}

int
evas_filter_smallest_pow2_larger_than(int val)
{
   int n;

   for (n = 0; n < 32; n++)
     if (val <= (1 << n)) return n;

   ERR("Value %d is too damn high!", val);
   return 32;
}
