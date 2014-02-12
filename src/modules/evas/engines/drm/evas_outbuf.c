#include "evas_engine.h"
#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

/* FIXME: We NEED to get the color map from the VT and use that for the mask */
#define RED_MASK 0x00ff0000
#define GREEN_MASK 0x0000ff00
#define BLUE_MASK 0x000000ff

static Eina_Bool 
_evas_outbuf_buffer_new(Outbuf *ob, Buffer *buffer)
{
   /* check for valid outbuf */
   if (!ob) return EINA_FALSE;

   buffer->w = ob->w;
   buffer->h = ob->h;

   /* sadly we cannot create buffers to be JUST the size of the canvas.
    * drmModeSetCrtc will barf with ENOSPC if we try that :( so we have to 
    * allocate framebuffer objects to be the whole size of the display mode */

   /* if (ob->priv.mode.hdisplay > buffer->w) */
   /*   buffer->w = ob->priv.mode.hdisplay; */
   /* if (ob->priv.mode.vdisplay > buffer->h) */
   /*   buffer->h = ob->priv.mode.vdisplay; */

   /* create a drm fb for this buffer */
   if (!evas_drm_framebuffer_create(ob->priv.fd, buffer, ob->depth))
     {
        CRI("Could not create drm framebuffer");
        return EINA_FALSE;
     }

   evas_drm_outbuf_framebuffer_set(ob, buffer);

   return EINA_TRUE;
}

static void *
_evas_outbuf_buffer_map(Outbuf *ob, int *w, int *h)
{
   if (w) *w = ob->w;
   if (h) *h = ob->h;
   return ob->priv.buffer[ob->priv.curr].data;
}

static void 
_evas_outbuf_buffer_put(Outbuf *ob, Buffer *buffer, Eina_Rectangle *rects, unsigned int count)
{
   /* validate input params */
   if ((!ob) || (!buffer)) return;

   if (!buffer->data)
     {
        CRI("Current Buffer Has No Data !!");
        /* TODO: call function to mmap buffer data */
        /* if (!_evas_outbuf_buffer_new(ob, buffer)); */
        return;
     }

   if (ob->priv.sent != buffer)
     {
        /* DBG("Send New Buffer: %d", buffer->fb); */
        /* if (!buffer->valid) evas_drm_outbuf_framebuffer_set(ob, buffer); */
        if (!evas_drm_framebuffer_send(ob, buffer, rects, count))
          ERR("Could not send buffer");
     }
}

static void 
_evas_outbuf_buffer_swap(Outbuf *ob, Eina_Rectangle *rects, unsigned int count)
{
   /* check for valid output buffer */
   if (!ob) return;

   _evas_outbuf_buffer_put(ob, &(ob->priv.buffer[ob->priv.curr]), 
                           rects, count);
}

Outbuf *
evas_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h)
{
   Outbuf *ob;
   char *num;
   int i = 0;

   /* try to allocate space for our outbuf structure */
   if (!(ob = calloc(1, sizeof(Outbuf))))
     return NULL;

   /* set some default outbuf properties */
   ob->w = w;
   ob->h = h;
   ob->rotation = info->info.rotation;
   ob->depth = info->info.depth;
   ob->destination_alpha = info->info.destination_alpha;

   /* set drm file descriptor */
   ob->priv.fd = info->info.fd;

   /* try to setup the drm card for this outbuf */
   if (!evas_drm_outbuf_setup(ob))
     {
        free(ob);
        return NULL;
     }

   ob->priv.num = NUM_BUFFERS;

   /* check for buffer override */
   if ((num = getenv("EVAS_DRM_BUFFERS")))
     {
        ob->priv.num = atoi(num);

        /* cap maximum # of buffers */
        if (ob->priv.num <= 0) ob->priv.num = 1;
        else if (ob->priv.num > 3) ob->priv.num = 3;
     }

   /* with the connector and crtc set, we can now create buffers !! :) */
   for (; i < ob->priv.num; i++)
     {
        if (!_evas_outbuf_buffer_new(ob, &(ob->priv.buffer[i])))
          {
             CRI("Failed to create buffer");
             break;
          }
     }

   /* set array step size for regions */
   eina_array_step_set(&ob->priv.onebuf_regions, sizeof(Eina_Array), 8);

   /* return the allocated outbuf structure */
   return ob;
}

void 
evas_outbuf_free(Outbuf *ob)
{
   int i = 0;

   /* destroy the buffers */
   for (; i < ob->priv.num; i++)
     evas_drm_framebuffer_destroy(ob->priv.fd, &(ob->priv.buffer[i]));

   eina_array_flush(&ob->priv.onebuf_regions);

   /* free the allocated outbuf structure */
   free(ob);
}

void 
evas_outbuf_reconfigure(Evas_Engine_Info_Drm *info, Outbuf *ob, int w, int h)
{
   int i = 0;

   /* check for changes */
   if ((ob->w == w) && (ob->h == h) && 
       (ob->destination_alpha == info->info.destination_alpha) && 
       (ob->rotation == info->info.rotation) && 
       (ob->depth == info->info.depth)) 
     return;

   /* set new outbuf properties */
   ob->rotation =info->info. rotation;
   ob->depth = info->info.depth;
   ob->destination_alpha = info->info.destination_alpha;

   /* handle rotation */
   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        ob->w = w;
        ob->h = h;
     }
   else
     {
        ob->w = h;
        ob->h = w;
     }

   /* destroy the old buffers */
   for (; i < ob->priv.num; i++)
     evas_drm_framebuffer_destroy(ob->priv.fd, &(ob->priv.buffer[i]));

   for (i = 0; i < ob->priv.num; i++)
     {
        if (!_evas_outbuf_buffer_new(ob, &(ob->priv.buffer[i])))
          {
             CRI("Failed to create buffer");
             break;
          }
     }
}

int 
evas_outbuf_buffer_state_get(Outbuf *ob)
{
   int i = 0, n = 0, count = 0;

   /* check for valid output buffer */
   if (!ob) return MODE_FULL;

   for (; i < ob->priv.num; i++)
     {
        n = (ob->priv.num + ob->priv.curr - i) % ob->priv.num;
        if (ob->priv.buffer[n].valid) count++;
        else break;
     }

   if (count == ob->priv.num)
     {
        if (count == 1) return MODE_COPY;
        else if (count == 2) return MODE_DOUBLE;
        else if (count == 3) return MODE_TRIPLE;
     }

   return MODE_FULL;
}

RGBA_Image *
evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *img;
   Eina_Rectangle *rect;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, ob->w, ob->h);
   if ((w <= 0) || (h <= 0)) return NULL;

   if (ob->rotation == 0)
     {
        if (!(img = ob->priv.onebuf))
          {
             int bpl = 0;
             int bw = 0, bh = 0;
             void *data;

             data = _evas_outbuf_buffer_map(ob, &bw, &bh);
             bpl = (bw * sizeof(int));

#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               img = (RGBA_Image *)evas_cache2_image_data(evas_common_image_cache2_get(),
                                                          bpl / sizeof(int), bh, 
                                                          data, 
                                                          ob->destination_alpha, 
                                                          EVAS_COLORSPACE_ARGB8888);
             else
#endif
               img = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(),
                                                         bpl / sizeof(int), bh, 
                                                         data, 
                                                         ob->destination_alpha, 
                                                         EVAS_COLORSPACE_ARGB8888);

             ob->priv.onebuf = img;
             if (!img) return NULL;
          }

        if (!(rect = eina_rectangle_new(x, y, w, h)))
          return NULL;

        if (!eina_array_push(&ob->priv.onebuf_regions, rect))
          {
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               evas_cache2_image_close(&img->cache_entry);
             else
#endif
               evas_cache_image_drop(&img->cache_entry);

             eina_rectangle_free(rect);

             return NULL;
          }

        /* clip the region to the onebuf region */
        if (cx) *cx = x;
        if (cy) *cy = y;
        if (cw) *cw = w;
        if (ch) *ch = h;

        return img;
     }
   else
     {
        if (!(rect = eina_rectangle_new(x, y, w, h)))
          return NULL;

#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          img = (RGBA_Image *)evas_cache2_image_empty(evas_common_image_cache2_get());
        else
#endif
          img = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());

        if (!img)
          {
             eina_rectangle_free(rect);
             return NULL;
          }

        img->cache_entry.flags.alpha = ob->destination_alpha;
//        img->cache_entry.flags.alpha |= ob->destination_alpha;

#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_surface_alloc(&img->cache_entry, w, h);
        else
#endif
          evas_cache_image_surface_alloc(&img->cache_entry, w, h);

        img->extended_info = rect;

        ob->priv.pending_writes = 
          eina_list_append(ob->priv.pending_writes, img);

        if (cx) *cx = 0;
        if (cy) *cy = 0;
        if (cw) *cw = w;
        if (ch) *ch = h;

        return img;
     }

   return NULL;
}

void 
evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert func = NULL;
   Eina_Rectangle rect = {0, 0, 0, 0}, pr;
   DATA32 *src;
   DATA8 *dst;
   int depth = 32, bpp = 0, bpl = 0, wid = 0;
   int ww = 0, hh = 0;
   int rx = 0, ry = 0;

   /* check for valid output buffer */
   if (!ob) return;

   /* check for pending writes */
   if (!ob->priv.pending_writes) return;

   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        func = 
          evas_common_convert_func_get(0, w, h, depth, 
                                       RED_MASK, GREEN_MASK, BLUE_MASK,
                                       PAL_MODE_NONE, ob->rotation);
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        func = 
          evas_common_convert_func_get(0, h, w, depth, 
                                       RED_MASK, GREEN_MASK, BLUE_MASK,
                                       PAL_MODE_NONE, ob->rotation);
     }

   /* make sure we have a valid convert function */
   if (!func) return;

   /* based on rotation, set rectangle position */
   if (ob->rotation == 0)
     {
        rect.x = x;
        rect.y = y;
     }
   else if (ob->rotation == 90)
     {
        rect.x = y;
        rect.y = (ob->w - x - w);
     }
   else if (ob->rotation == 180)
     {
        rect.x = (ob->w - x - w);
        rect.y = (ob->h - y - h);
     }
   else if (ob->rotation == 270)
     {
        rect.x = (ob->h - y - h);
        rect.y = x;
     }

   /* based on rotation, set rectangle size */
   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        rect.w = w;
        rect.h = h;
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        rect.w = h;
        rect.h = w;
     }

   /* check for valid update image data */
   if (!(src = update->image.data)) return;

   bpp = depth / 8;
   if (bpp <= 0) return;

   /* check for valid desination data */
   if (!(dst = _evas_outbuf_buffer_map(ob, &ww, &hh))) return;

   bpl = (ww * sizeof(int));

   if (ob->rotation == 0)
     {
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 0, 0, ww, hh);
        dst += (bpl * rect.y) + (rect.x + bpp);
        w -= rx;
     }
   else if (ob->rotation == 180)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 0, 0, ww, hh);
        rx = pr.w - rect.w;
        ry = pr.h - rect.h;
        src += (update->cache_entry.w * ry) + rx;
        w -= rx;
     }
   else if (ob->rotation == 90)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 0, 0, ww, hh);
        rx = pr.w - rect.w; ry = pr.h - rect.h;
        src += ry;
        w -= ry;
     }
   else if (ob->rotation == 270)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 0, 0, ww, hh);
        rx = pr.w - rect.w; ry = pr.h - rect.h;
        src += (update->cache_entry.w * rx);
        w -= ry;
     }

   if ((rect.w <= 0) || (rect.h <= 0)) return;

   wid = bpl / bpp;

   dst += (bpl * rect.y) + (rect.x * bpp);

   func(src, dst, (update->cache_entry.w - w), (wid - rect.w),
        rect.w, rect.h, x + rx, y + ry, NULL);
}

void 
evas_outbuf_flush(Outbuf *ob)
{
   Eina_Rectangle *rects;
   RGBA_Image *img;
   unsigned int n = 0, i = 0;

   /* check for valid output buffer */
   if (!ob) return;

   /* check for pending writes */
   if (!ob->priv.pending_writes)
     {
        Eina_Rectangle *rect;
        Eina_Array_Iterator it;

        /* get number of buffer regions */
        n = eina_array_count_get(&ob->priv.onebuf_regions);
        if (n == 0) return;

        /* allocate rectangles */
        if (!(rects = alloca(n * sizeof(Eina_Rectangle)))) return;

        /* loop the buffer regions and assign to rects */
        EINA_ARRAY_ITER_NEXT(&ob->priv.onebuf_regions, i, rect, it)
          rects[i] = *rect;

        /* TODO: unmap the buffer ?? */
        /* evas_swapper_buffer_unmap(ob->priv.swapper); */

        /* force a buffer swap */
        _evas_outbuf_buffer_swap(ob, rects, n);

        /* clean array */
        eina_array_clean(&ob->priv.onebuf_regions);

        img = ob->priv.onebuf;
        ob->priv.onebuf = NULL;
        if (img)
          {
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               evas_cache2_image_close(&img->cache_entry);
             else
#endif
               evas_cache_image_drop(&img->cache_entry);
          }
     }
   else
     {
        /* get number of pending writes */
        n = eina_list_count(ob->priv.pending_writes);
        if (n == 0) return;

        /* allocate rectangles */
        if (!(rects = alloca(n * sizeof(Eina_Rectangle)))) return;

        /* loop the pending writes */
        EINA_LIST_FREE(ob->priv.pending_writes, img)
          {
             Eina_Rectangle *rect;
             int x = 0, y = 0, w = 0, h = 0;

             if (!(rect = img->extended_info)) continue;

             x = rect->x; y = rect->y; w = rect->w; h = rect->h;

             /* based on rotation, set rectangle position */
             if (ob->rotation == 0)
               {
                  rects[i].x = x;
                  rects[i].y = y;
               }
             else if (ob->rotation == 90)
               {
                  rects[i].x = y;
                  rects[i].y = (ob->w - x - w);
               }
             else if (ob->rotation == 180)
               {
                  rects[i].x = (ob->w - x - w);
                  rects[i].y = (ob->h - y - h);
               }
             else if (ob->rotation == 270)
               {
                  rects[i].x = (ob->h - y - h);
                  rects[i].y = x;
               }

             /* based on rotation, set rectangle size */
             if ((ob->rotation == 0) || (ob->rotation == 180))
               {
                  rects[i].w = w;
                  rects[i].h = h;
               }
             else if ((ob->rotation == 90) || (ob->rotation == 270))
               {
                  rects[i].w = h;
                  rects[i].h = w;
               }

             eina_rectangle_free(rect);

#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               evas_cache2_image_close(&img->cache_entry);
             else
#endif
               evas_cache_image_drop(&img->cache_entry);

             i++;
          }

        /* TODO: unmap the buffer ?? */
        /* evas_swapper_buffer_unmap(ob->priv.swapper); */

        /* force a buffer swap */
        _evas_outbuf_buffer_swap(ob, rects, n);
     }
}
