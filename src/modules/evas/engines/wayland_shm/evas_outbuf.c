#include "evas_common_private.h"
#include "evas_private.h"
#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif
#include "evas_engine.h"

#define RED_MASK 0x00ff0000
#define GREEN_MASK 0x0000ff00
#define BLUE_MASK 0x000000ff

Outbuf *
_evas_outbuf_setup(int w, int h, int rot, Outbuf_Depth depth, Eina_Bool alpha, struct wl_shm *shm, struct wl_surface *surface)
{
   Outbuf *ob = NULL;
   char *num;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to allocate space for new Outbuf */
   if (!(ob = calloc(1, sizeof(Outbuf)))) return NULL;

   /* set outbuf properties */
   ob->w = w;
   ob->h = h;
   ob->rotation = rot;
   ob->depth = depth;
   ob->priv.destination_alpha = alpha;

   /* default to double buffer */
   ob->num_buff = 2;

   /* check for any 'number of buffers' override in the environment */
   if ((num = getenv("EVAS_WAYLAND_SHM_BUFFERS")))
     {
        int n = 0;

        n = atoi(num);
        if (n <= 0) n = 1;
        if (n > 4) n = 4;

        ob->num_buff = n;
     }

   /* try to create the outbuf surface */
   if (!(ob->surface = _evas_shm_surface_create(shm, surface, w, h, alpha)))
     goto surf_err;

   /* call prepare function to setup first buffer */
   _evas_shm_surface_prepare(ob->surface, 0, 0, w, h, 
                             ob->num_buff, ob->surface->flags);

   eina_array_step_set(&ob->priv.onebuf_regions, sizeof(Eina_Array), 8);

   return ob;

surf_err:
   free(ob);
   return NULL;
}

void 
_evas_outbuf_free(Outbuf *ob)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _evas_outbuf_flush(ob, NULL, MODE_FULL);
   _evas_outbuf_idle_flush(ob);

   if (ob->surface) _evas_shm_surface_destroy(ob->surface);

   eina_array_flush(&ob->priv.onebuf_regions);

   free(ob);
}

void 
_evas_outbuf_idle_flush(Outbuf *ob)
{
   _evas_shm_surface_redraw(ob->surface);
}

void 
_evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Eina_Rectangle *result;
   RGBA_Image *img;
   unsigned int n = 0, i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (!ob->surface->current)
     {
        WRN("Cannot Flush. No Current Leaf !!");
        return;
     }

   /* check for pending writes */
   if (!ob->priv.pending_writes)
     {
        Eina_Rectangle *rect;
        Eina_Array_Iterator it;

        /* get number of buffer regions */
        n = eina_array_count_get(&ob->priv.onebuf_regions);
        if (n == 0) return;

        /* allocate rectangles */
        if (!(result = alloca(n * sizeof(Eina_Rectangle)))) return;

        /* loop the buffer regions and assign to result */
        EINA_ARRAY_ITER_NEXT(&ob->priv.onebuf_regions, i, rect, it)
          result[i] = *rect;

        /* force a buffer swap */
        _evas_shm_surface_swap(ob->surface, result, n);

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
        if (!(result = alloca(n * sizeof(Eina_Rectangle)))) return;

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
                  result[i].x = x;
                  result[i].y = y;
               }
             else if (ob->rotation == 90)
               {
                  result[i].x = y;
                  result[i].y = (ob->w - x - w);
               }
             else if (ob->rotation == 180)
               {
                  result[i].x = (ob->w - x - w);
                  result[i].y = (ob->h - y - h);
               }
             else if (ob->rotation == 270)
               {
                  result[i].x = (ob->h - y - h);
                  result[i].y = x;
               }

             /* based on rotation, set rectangle size */
             if ((ob->rotation == 0) || (ob->rotation == 180))
               {
                  result[i].w = w;
                  result[i].h = h;
               }
             else if ((ob->rotation == 90) || (ob->rotation == 270))
               {
                  result[i].w = h;
                  result[i].h = w;
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

        /* force a buffer swap */
        _evas_shm_surface_swap(ob->surface, result, n);
     }

   _evas_shm_surface_redraw(ob->surface);
}

Render_Engine_Swap_Mode 
_evas_outbuf_swapmode_get(Outbuf *ob)
{
   int i = 0, count = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   for (; i < ob->num_buff; i++)
     {
        if (ob->surface->leaf[i].busy)
          count++;
     }

   switch (count)
     {
      case 0:
        return MODE_COPY;
      case 1:
        return MODE_DOUBLE;
      case 2:
        return MODE_TRIPLE;
      case 3:
        return MODE_QUADRUPLE;
      default:
        return MODE_FULL;
     }
}

int 
_evas_outbuf_rotation_get(Outbuf *ob)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ob->rotation;
}

void 
_evas_outbuf_reconfigure(Outbuf *ob, int x, int y, int w, int h, int rot, Outbuf_Depth depth, Eina_Bool alpha, Eina_Bool resize)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (depth == OUTBUF_DEPTH_INHERIT) depth = ob->depth;

   if ((ob->w == w) && (ob->h == h) && 
       (ob->rotation == rot) && (ob->depth == depth) && 
       (ob->priv.destination_alpha == alpha))
     return;

   ob->w = w;
   ob->h = h;
   ob->rotation = rot;
   ob->depth = depth;
   ob->priv.destination_alpha = alpha;

   if (resize)
     ob->surface->flags = SURFACE_HINT_RESIZING;
   else
     ob->surface->flags = 0;

   _evas_shm_surface_prepare(ob->surface, x, y, w, h, 
                             ob->num_buff, ob->surface->flags);
}

void *
_evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
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
             int bw = 0, bh = 0, bpl = 0;
             void *data;

             if (!(data = _evas_shm_surface_data_get(ob->surface, &bw, &bh)))
               {
                  ERR("Could not get surface data");
                  return NULL;
               }

             bpl = (bw * sizeof(int));

#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               {
                  img = (RGBA_Image *)
                    evas_cache2_image_data(evas_common_image_cache2_get(),
                                           bpl / sizeof(int), bh, data,
                                           ob->priv.destination_alpha,
                                           EVAS_COLORSPACE_ARGB8888);
               }
             else
#endif
               {
                  img = (RGBA_Image *)
                    evas_cache_image_data(evas_common_image_cache_get(),
                                          bpl / sizeof(int), bh, data,
                                          ob->priv.destination_alpha,
                                          EVAS_COLORSPACE_ARGB8888);

               }

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

        if (cx) *cx = x;
        if (cy) *cy = y;
        if (cw) *cw = w;
        if (ch) *ch = h;

        img->extended_info = rect;

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

        img->cache_entry.flags.alpha |= ob->priv.destination_alpha ? 1 : 0;

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
_evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h)
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
   if (!(dst = _evas_shm_surface_data_get(ob->surface, &ww, &hh))) return;

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
_evas_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}
