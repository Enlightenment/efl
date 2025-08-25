#include "evas_engine.h"

Drm_Swapper *
_drm_swapper_new(Ecore_Drm2_Device *dev, int w, int h, int depth, int bpp, unsigned int format)
{
   Drm_Swapper *swp;

   swp = calloc(1, sizeof(Drm_Swapper));
   if (!swp) return NULL;

   swp->w = w;
   swp->h = h;
   swp->dev = dev;
   swp->bpp = bpp;
   swp->depth = depth;
   swp->format = format;
   swp->last_count = -1;

   return swp;
}

void
_drm_swapper_free(Drm_Swapper *swp)
{
   Drm_Buffer *buf;

   if (swp->mapped) _drm_swapper_buffer_unmap(swp);
   EINA_LIST_FREE(swp->cache, buf)
     {
        ecore_drm2_fb_destroy(buf->fb);
        free(buf);
     }

   free(swp);
}

void *
_drm_swapper_buffer_map(Drm_Swapper *swp, int *bpl, int *w, int *h)
{
   Drm_Buffer *buf;
   Eina_List *l;

   if (swp->mapped)
     {
        if (bpl)
          {
             if (swp->buf)
               *bpl = ecore_drm2_fb_stride_get(swp->buf->fb);
             else
               *bpl = swp->w * 4;
          }
        if (w) *w = swp->w;
        if (h) *h = swp->h;

        return ecore_drm2_fb_map_get(swp->buf_fb);
     }

   if ((swp->buf) && (!swp->buf->reused))
     {
        EINA_LIST_FREE(swp->cache, buf)
          {
             ecore_drm2_fb_destroy(buf->fb);
             free(buf);
          }
     }
   else
     {
        EINA_LIST_FOREACH(swp->cache, l, buf)
          {
             if (buf == swp->buf)
               {
                  buf->reused = EINA_TRUE;
                  swp->buf = buf;
                  swp->buf_fb = buf->fb;
                  swp->cache = eina_list_promote_list(swp->cache, l);
                  break;
               }
          }
     }

   if (!swp->buf_fb)
     {
        buf = calloc(1, sizeof(Drm_Buffer));
        if (!buf) return NULL;

        buf->fb =
          ecore_drm2_fb_create(swp->dev, swp->w, swp->h, swp->depth,
                               swp->bpp, swp->format, NULL);
        if (!buf->fb)
          {
             ERR("\tCould Not Create Fb");
             free(buf);
             return NULL;
          }

        buf->index = eina_list_count(swp->cache);

        swp->buf = buf;
        swp->buf_fb = buf->fb;
        swp->cache = eina_list_prepend(swp->cache, buf);

        /* trim buffer cache */
        while (eina_list_count(swp->cache) > 4)
          {
             l = eina_list_last(swp->cache);
             if (l)
               {
                  buf = l->data;
                  swp->cache = eina_list_remove_list(swp->cache, l);
                  ecore_drm2_fb_destroy(buf->fb);
                  free(buf);
               }
          }
     }

   if (!ecore_drm2_fb_map(swp->buf_fb))
     {
        ERR("Failed to map swap buffer: %m");
        return NULL;
     }

   if (bpl) *bpl = ecore_drm2_fb_stride_get(swp->buf_fb);
   if (w) *w = swp->w;
   if (h) *h = swp->h;

   swp->mapped = EINA_TRUE;
   if (swp->buf) swp->buf->mapped = EINA_TRUE;

   return ecore_drm2_fb_map_get(swp->buf_fb);
}

void
_drm_swapper_buffer_unmap(Drm_Swapper *swp)
{
   if (!swp->mapped) return;

   ecore_drm2_fb_unmap(swp->buf_fb);
   free(swp->buf);

   swp->buf = NULL;
   swp->buf_fb = NULL;
   swp->mapped = EINA_FALSE;
}

void
_drm_swapper_swap(Drm_Swapper *swp EINA_UNUSED, Eina_Rectangle *rects EINA_UNUSED, int num EINA_UNUSED)
{
   /* int i = 0; */

   /* for (; i < num; i++) */
   /*   { */
   /*   } */

   /* TODO */
   /* create region */
   /* swap buffers with region */
   /* destroy region */
}

Render_Output_Swap_Mode
_drm_swapper_mode_get(Drm_Swapper *swp)
{
   if (!swp->mapped) _drm_swapper_buffer_map(swp, NULL, NULL, NULL);

   if (!swp->mapped) return MODE_FULL;
   if (!swp->buf) return MODE_FULL;

   if (swp->buf->index != swp->last_count)
     {
        swp->last_count = swp->buf->index;
        return MODE_FULL;
     }

   if (swp->buf->index == 0)
     return MODE_FULL;
   else if (swp->buf->index == 1)
     return MODE_COPY;
   else if (swp->buf->index == 2)
     return MODE_DOUBLE;
   else if (swp->buf->index == 3)
     return MODE_TRIPLE;
   else if (swp->buf->index == 4)
     return MODE_QUADRUPLE;

   return MODE_FULL;
}

void
_drm_swapper_dirty(Drm_Swapper *swp, Eina_Rectangle *rects, int num)
{
   ecore_drm2_fb_dirty(swp->buf_fb, rects, num);
}
