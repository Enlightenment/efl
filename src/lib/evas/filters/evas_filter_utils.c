/* Utility functions for the filters.  */

#include "evas_filter_private.h"

Evas_Filter_Buffer *
evas_filter_buffer_scaled_get(Evas_Filter_Context *ctx,
                              Evas_Filter_Buffer *src,
                              unsigned w, unsigned h)
{
   Evas_Filter_Buffer *fb;
   Image_Entry *dstdata = NULL;
   Image_Entry *srcdata;
   void *drawctx;

   srcdata = evas_filter_buffer_backing_get(ctx, src->id);
   EINA_SAFETY_ON_NULL_RETURN_VAL(srcdata, NULL);

   fb = evas_filter_temporary_buffer_get(ctx, w, h, src->alpha_only);
   if (!fb) return NULL;

   dstdata = evas_filter_buffer_backing_get(ctx, fb->id);
   if (!dstdata)
     {
        CRI("No backing found for buffer %d", fb->id);
        return NULL;
     }

   if ((dstdata->w != w) || (dstdata->h != h))
     {
        CRI("Buffer size mismatch: got %dx%d requested %dx%d",
            dstdata->w, dstdata->h, w, h);
        return NULL;
     }

   // FIXME: Not supported on GL engine.
   // Yeah, we need to call the CPU scaling functions and not the engine.
   if (ctx->gl_engine)
     {
        if (src->w == (int) w && src->h == (int) h)
          {
             RGBA_Image *s = (RGBA_Image *) srcdata;
             RGBA_Image *d = (RGBA_Image *) dstdata;
             EINA_SAFETY_ON_NULL_RETURN_VAL(s->image.data, NULL);
             EINA_SAFETY_ON_NULL_RETURN_VAL(d->image.data, NULL);
             memcpy(d->image.data, s->image.data,
                    w * h * (src->alpha_only ? 1 : 4));
          }
        else
          {
             CRI("Support for stretching not implemened yet for GL.");
             return NULL;
          }
     }
   else if (!src->alpha_only)
     {
        drawctx = ENFN->context_new(ENDT);
        ENFN->context_color_set(ENDT, drawctx, 255, 255, 255, 255);
        ENFN->context_render_op_set(ENDT, drawctx, EVAS_RENDER_COPY);
        ENFN->image_draw(ENDT, drawctx, dstdata, srcdata,
                         0, 0, src->w, src->h, // src
                         0, 0, w, h, // dst
                         EINA_TRUE, // smooth
                         EINA_FALSE); // Not async
        ENFN->context_free(ENDT, drawctx);
     }
   else
     {
        // FIXME: How to scale alpha buffer?
        // For now, we could draw to RGBA, scale from there, draw back to alpha
        CRI("Alpha buffer scaling is not supported");
        return NULL;
     }

   return fb;
}

static Eina_Bool
_interpolate_none(DATA8 *output, DATA8 *points, int point_count)
{
   int j, k, val, x1, x2;
   for (j = 0; j < point_count; j++)
     {
        x1 = points[j * 2];
        val = points[j * 2 + 1];
        if (j < (point_count - 1))
          x2 = points[(j + 1) * 2];
        else
          x2 = 256;
        if (x2 < x1) return EINA_FALSE;
        for (k = x1; k < x2; k++)
          output[k] = val;
     }
   return EINA_TRUE;
}

static Eina_Bool
_interpolate_linear(DATA8 *output, DATA8 *points, int point_count)
{
   int j, k, val1, val2, x1, x2;
   for (j = 0; j < point_count; j++)
     {
        x1 = points[j * 2];
        val1 = points[j * 2 + 1];
        if (j < (point_count - 1))
          {
             x2 = points[(j + 1) * 2];
             val2 = points[(j + 1) * 2 + 1];
          }
        else
          {
             x2 = 256;
             val2 = val1;
          }
        if (x2 < x1) return EINA_FALSE;
        for (k = x1; k < x2; k++)
          output[k] = val1 + ((val2 - val1) * (k - x1)) / (x2 - x1);
     }
   return EINA_TRUE;
}

Eina_Bool
evas_filter_interpolate(DATA8 *output, DATA8 *points, int point_count,
                        Evas_Filter_Interpolation_Mode mode)
{
   switch (mode)
     {
      case EVAS_FILTER_INTERPOLATION_MODE_NONE:
        return _interpolate_none(output, points, point_count);
      case EVAS_FILTER_INTERPOLATION_MODE_LINEAR:
      default:
        return _interpolate_linear(output, points, point_count);
     }
}
