/** @file evas_filter_utils.c
 * @brief Utility functions for the filters.
 */

#include "evas_filter_private.h"

Evas_Filter_Buffer *
evas_filter_buffer_scaled_get(Evas_Filter_Context *ctx,
                              Evas_Filter_Buffer *src,
                              int w, int h)
{
   Evas_Filter_Buffer *fb;
   void *dstdata = NULL;
   void *srcdata;
   void *drawctx;

   srcdata = evas_filter_buffer_backing_get(ctx, src->id);
   EINA_SAFETY_ON_NULL_RETURN_VAL(srcdata, NULL);

   fb = evas_filter_temporary_buffer_get(ctx, w, h, src->alpha_only);
   if (!fb) return NULL;

   if (evas_filter_buffer_alloc(fb, w, h))
     dstdata = evas_filter_buffer_backing_get(ctx, fb->id);
   if (!dstdata)
     {
        ERR("Buffer allocation failed for size %dx%d", w, h);
        return NULL;
     }

   if (!src->alpha_only)
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
