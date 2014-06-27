#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

//#include <sys/mman.h>

#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

#include "evas_common_private.h"
#include "evas_macros.h"
#include "evas_engine.h"
#include "evas_swapbuf.h"
#include "evas_swapper.h"

#define RED_MASK 0x00ff0000
#define GREEN_MASK 0x0000ff00
#define BLUE_MASK 0x000000ff

/* local function prototypes */

Outbuf *
evas_swapbuf_setup(Evas_Engine_Info_Wayland_Shm *info, int w, int h, unsigned int rotation, Outbuf_Depth depth, Eina_Bool alpha, struct wl_shm *wl_shm, struct wl_surface *wl_surface)
{
   Outbuf *ob = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to allocate a new Outbuf */
   if (!(ob = calloc(1, sizeof(Outbuf)))) 
     return NULL;

   /* set some properties */
   ob->w = w;
   ob->h = h;
   ob->rotation = rotation;
   ob->depth = depth;
   ob->priv.destination_alpha = alpha;
   ob->priv.wl.shm = wl_shm;
   ob->priv.wl.surface = wl_surface;
   ob->info = info;

   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        ob->priv.swapper = 
          evas_swapper_setup(0, 0, w, h, depth, alpha, ob->priv.wl.shm,
                             ob->priv.wl.surface);
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        ob->priv.swapper = 
          evas_swapper_setup(0, 0, h, w, depth, alpha, ob->priv.wl.shm,
                             ob->priv.wl.surface);
     }

   /* check that a swapper was created */
   if (!ob->priv.swapper)
     {
        /* free the Outbuf structure allocation */
        free(ob);

        return NULL;
     }

   /* set step size of regions array */
   eina_array_step_set(&ob->priv.onebuf_regions, sizeof(Eina_Array), 8);

   /* return allocated Outbuf */
   return ob;
}

void 
evas_swapbuf_free(Outbuf *ob)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid output buffer */
   if (!ob) return;

   /* flush the output buffer */
   evas_swapbuf_flush(ob);
   evas_swapbuf_idle_flush(ob);
   evas_swapper_free(ob->priv.swapper);
   eina_array_flush(&ob->priv.onebuf_regions);

   /* free the allocated structure */
   free(ob);
}

void 
evas_swapbuf_reconfigure(Outbuf *ob, int x, int y, int w, int h, int rotation, Outbuf_Depth depth, Eina_Bool alpha)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid output buffer */
   if (!ob) return;

   /* check that something was actually changed */
   if ((ob->w == w) && (ob->h == h) && 
       (ob->rotation == rotation) && (ob->depth == depth)) 
     return;

   /* set some properties */
   ob->w = w;
   ob->h = h;
   ob->rotation = rotation;
   ob->depth = depth;
   ob->priv.destination_alpha = alpha;

   /* check for valid swapper */
   if (ob->priv.swapper)
     {
        if ((ob->rotation == 0) || (ob->rotation == 180))
          ob->priv.swapper = evas_swapper_reconfigure(ob->priv.swapper,
                                                      x, y, w, h, depth,
                                                      alpha);
        else if ((ob->rotation == 90) || (ob->rotation == 270))
          ob->priv.swapper = evas_swapper_reconfigure(ob->priv.swapper,
                                                      x, y, h, w, depth,
                                                      alpha);
        return;
     }

   /* create new swapper */
   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        ob->priv.swapper = 
          evas_swapper_setup(x, y, w, h, depth, alpha, ob->priv.wl.shm, 
                             ob->priv.wl.surface);
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        ob->priv.swapper = 
          evas_swapper_setup(x, y, h, w, depth, alpha, ob->priv.wl.shm, 
                             ob->priv.wl.surface);
     }
}

RGBA_Image *
evas_swapbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *img;
   Eina_Rectangle *rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, ob->w, ob->h);
   if ((w <= 0) || (h <= 0)) return NULL;

   if (ob->rotation == 0)
     {
        if (!(img = ob->priv.onebuf))
          {
             int bpl = 0;
             int bw = 0, bh = 0;
             void *data;

             data = evas_swapper_buffer_map(ob->priv.swapper, &bw, &bh);
             bpl = (bw * sizeof(int));

#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               img = (RGBA_Image *)evas_cache2_image_data(evas_common_image_cache2_get(),
                                                          bpl / sizeof(int), bh, 
                                                          data, 
                                                          ob->priv.destination_alpha, 
                                                          EVAS_COLORSPACE_ARGB8888);
             else
#endif
               img = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(),
                                                         bpl / sizeof(int), bh, 
                                                         data, 
                                                         ob->priv.destination_alpha, 
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

        img->cache_entry.flags.alpha |= ob->priv.destination_alpha;

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
evas_swapbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert func = NULL;
   Eina_Rectangle rect = {0, 0, 0, 0}, pr;
   DATA32 *src;
   DATA8 *dst;
   int depth = 32, bpp = 0, bpl = 0, wid = 0;
   int ww = 0, hh = 0;
   int rx = 0, ry = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

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
   if (!(dst = evas_swapper_buffer_map(ob->priv.swapper, &ww, &hh))) return;

   bpl = (ww * sizeof(int));

   if (ob->rotation == 0)
     {
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 0, 0, ww, hh);
        dst += (bpl * rect.y) + (rect.x * bpp);
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
evas_swapbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{
   /* NB: nothing to do, they are cleaned up on flush */
}

void 
evas_swapbuf_flush(Outbuf *ob)
{
   Eina_Rectangle *rects;
   RGBA_Image *img;
   unsigned int n = 0, i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

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

        /* unmap the buffer */
        evas_swapper_buffer_unmap(ob->priv.swapper);

        /* force a buffer swap */
        evas_swapper_swap(ob->priv.swapper, rects, n);

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

        /* unmap the buffer */
        evas_swapper_buffer_unmap(ob->priv.swapper);

        /* force a buffer swap */
        evas_swapper_swap(ob->priv.swapper, rects, n);
     }
}

void 
evas_swapbuf_idle_flush(Outbuf *ob EINA_UNUSED)
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid output buffer */
   /* if (!ob) return; */

   /* check for valid swapper */
   /* if (!ob->priv.swapper) return; */

   /* tell the swapper to release any buffers that have been rendered */
   /* evas_swapper_buffer_idle_flush(ob->priv.swapper); */
}

Render_Engine_Swap_Mode
evas_swapbuf_state_get(Outbuf *ob)
{
   int mode = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ob->priv.swapper) return MODE_FULL;
   mode = evas_swapper_buffer_state_get(ob->priv.swapper);
   return mode;
}

int
evas_swapbuf_rotation_get(Outbuf *ob)
{
   return ob->rotation;
}

/* local functions */
