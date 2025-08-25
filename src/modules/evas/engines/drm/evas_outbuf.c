#include "evas_engine.h"

#define RED_MASK 0xff0000
#define GREEN_MASK 0x00ff00
#define BLUE_MASK 0x0000ff

Outbuf *
_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h)
{
   Outbuf *ob;

   ob = calloc(1, sizeof(Outbuf));
   if (!ob) return NULL;

   ob->w = w;
   ob->h = h;
   ob->info = info;

   if ((ob->info->rotation == 0) || (ob->info->rotation == 180))
     {
        ob->swapper =
          _drm_swapper_new(ob->info->dev, w, h, info->depth, info->bpp,
                           info->format);
     }
   else if ((ob->info->rotation == 90) || (ob->info->rotation == 270))
     {
        ob->swapper =
          _drm_swapper_new(ob->info->dev, h, w, info->depth, info->bpp,
                           info->format);
     }

   if (!ob->swapper)
     {
        free(ob);
        return NULL;
     }

   eina_array_step_set(&ob->onebuf_regions, sizeof(Eina_Array), 8);

   return ob;
}

void
_outbuf_free(Outbuf *ob)
{
   _outbuf_flush(ob, NULL, NULL, EVAS_RENDER_MODE_UNDEF);
   _outbuf_idle_flush(ob);

   _drm_swapper_free(ob->swapper);

   eina_array_flush(&ob->onebuf_regions);

   free(ob);
}

Render_Output_Swap_Mode
_outbuf_swap_mode_get(Outbuf *ob)
{
   return _drm_swapper_mode_get(ob->swapper);
}

int
_outbuf_rotation_get(Outbuf *ob)
{
   return ob->info->rotation;
}

void
_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth)
{
   unsigned int format = DRM_FORMAT_ARGB8888;

   switch (depth)
     {
      case OUTBUF_DEPTH_RGB_16BPP_565_565_DITHERED:
        format = DRM_FORMAT_RGB565;
        break;
      case OUTBUF_DEPTH_RGB_16BPP_555_555_DITHERED:
        format = DRM_FORMAT_RGBX5551;
        break;
      case OUTBUF_DEPTH_RGB_16BPP_444_444_DITHERED:
        format = DRM_FORMAT_RGBX4444;
        break;
      case OUTBUF_DEPTH_RGB_16BPP_565_444_DITHERED:
        format = DRM_FORMAT_RGB565;
        break;
      case OUTBUF_DEPTH_RGB_32BPP_888_8888:
        format = DRM_FORMAT_RGBX8888; // Should this be DRM_FORMAT_XRGB8888 ??
        break;
      case OUTBUF_DEPTH_ARGB_32BPP_8888_8888:
        format = DRM_FORMAT_ARGB8888;
        break;
      case OUTBUF_DEPTH_BGRA_32BPP_8888_8888:
        format = DRM_FORMAT_BGRA8888;
        break;
      case OUTBUF_DEPTH_BGR_32BPP_888_8888:
        format = DRM_FORMAT_BGRX8888;
        break;
      case OUTBUF_DEPTH_RGB_24BPP_888_888:
        format = DRM_FORMAT_RGB888;
        break;
      case OUTBUF_DEPTH_BGR_24BPP_888_888:
        format = DRM_FORMAT_BGR888;
        break;
      case OUTBUF_DEPTH_INHERIT:
      default:
        depth = ob->info->depth;
        format = ob->info->format;
        break;
     }

   if ((ob->w == w) && (ob->h == h) && (ob->info->rotation == rot) &&
       (ob->info->depth == depth) && (ob->info->format == format))
     return;

   ob->w = w;
   ob->h = h;
   ob->info->depth = depth;
   ob->info->format = format;
   ob->info->rotation = rot;

   _outbuf_idle_flush(ob);

   _drm_swapper_free(ob->swapper);

   if ((ob->info->rotation == 0) || (ob->info->rotation == 180))
     {
        ob->swapper =
          _drm_swapper_new(ob->info->dev, w, h, ob->info->depth, ob->info->bpp,
                           ob->info->format);
     }
   else if ((ob->info->rotation == 90) || (ob->info->rotation == 270))
     {
        ob->swapper =
          _drm_swapper_new(ob->info->dev, h, w, ob->info->depth, ob->info->bpp,
                           ob->info->format);
     }
}

void
_outbuf_damage_region_set(Outbuf *ob, Tilebuf_Rect *damage)
{
   Tilebuf_Rect *tr;
   Eina_Rectangle *rects;
   int count, i = 0;

   count = eina_inlist_count(EINA_INLIST_GET(damage));
   rects = alloca(count * sizeof(Eina_Rectangle));

   EINA_INLIST_FOREACH(damage, tr)
     {
        rects[i].x = tr->x;
        rects[i].y = tr->y;
        rects[i].w = tr->w;
        rects[i].h = tr->h;
        i++;
     }

   _drm_swapper_dirty(ob->swapper, rects, count);
}

void *
_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *img = NULL;
   Eina_Rectangle *rect;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, ob->w, ob->h);
   if ((w <= 0) || (h <= 0)) return NULL;

   if ((ob->info->rotation == 0))
     {
        Eina_Rectangle *rect;
        RGBA_Image *img;
        void *data;
        int bpl = 0;

        img = ob->onebuf;
        if (!img)
          {
             int ww = 0, hh = 0, bpp;

             /* bpp = ob->info->depth / 8; */
             bpp = ob->info->bpp;
             data = _drm_swapper_buffer_map(ob->swapper, &bpl, &ww, &hh);
             img = (RGBA_Image *)
               evas_cache_image_data(evas_common_image_cache_get(),
                                     bpl / bpp, hh, data, ob->alpha,
                                     EVAS_COLORSPACE_ARGB8888);
             ob->onebuf = img;
             if (!img) return NULL;
          }

        rect = eina_rectangle_new(x, y, w, h);
        if (!eina_array_push(&ob->onebuf_regions, rect))
          {
             evas_cache_image_drop(&img->cache_entry);
             eina_rectangle_free(rect);
             return NULL;
          }

        if (cx) *cx = x;
        if (cy) *cy = y;
        if (cw) *cw = w;
        if (ch) *ch = h;

        return img;
     }
   else
     {
        img = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        if (!img) return NULL;

        RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, ob->w, ob->h);

        if (!(rect = eina_rectangle_new(x, y, w, h)))
          return NULL;

        img->cache_entry.w = w;
        img->cache_entry.h = h;
        img->cache_entry.flags.alpha = ob->alpha;

        evas_cache_image_surface_alloc(&img->cache_entry, w, h);

        img->extended_info = rect;

        if (cx) *cx = 0;
        if (cy) *cy = 0;
        if (cw) *cw = w;
        if (ch) *ch = h;

        /* add this cached image data to pending update regions */
        ob->update_regions = eina_list_append(ob->update_regions, img);

        return img;
     }

   return NULL;
}

void
_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert conv_func = NULL;
   Eina_Rectangle rect = {0, 0, 0, 0}, pr;
   int bpp, bpl = 0, ww = 0, hh = 0;
   int rx = 0, ry = 0, wid;
   DATA32 *src;
   DATA8 *dst;

   if (!ob->update_regions) return;

   /* bpp = ob->info->depth / 8; */
   bpp = ob->info->bpp;
   if (bpp <= 0) return;

   if ((ob->info->rotation == 0) || (ob->info->rotation == 180))
     {
        conv_func =
          evas_common_convert_func_get(0, w, h, ob->info->depth,
                                       RED_MASK, GREEN_MASK, BLUE_MASK,
                                       PAL_MODE_NONE, ob->info->rotation);
     }
   else if ((ob->info->rotation == 90) || (ob->info->rotation == 270))
     {
        conv_func =
          evas_common_convert_func_get(0, h, w, ob->info->depth,
                                       RED_MASK, GREEN_MASK, BLUE_MASK,
                                       PAL_MODE_NONE, ob->info->rotation);
     }

   if (!conv_func) return;

   if (ob->info->rotation == 0)
     {
        rect.x = x;
        rect.y = y;
     }
   else if (ob->info->rotation == 90)
     {
        rect.x = y;
        rect.y = (ob->w - x - w);
     }
   else if (ob->info->rotation == 180)
     {
        rect.x = (ob->w = x - w);
        rect.y = (ob->h - y - h);
     }
   else if (ob->info->rotation == 270)
     {
        rect.x = (ob->h - y - h);
        rect.y = x;
     }

   if ((ob->info->rotation == 0) || (ob->info->rotation == 180))
     {
        rect.w = w;
        rect.h = h;
     }
   else if ((ob->info->rotation == 90) || (ob->info->rotation == 270))
     {
        rect.w = h;
        rect.h = w;
     }

   src = update->image.data;
   if (!src) return;

   dst = _drm_swapper_buffer_map(ob->swapper, &bpl, &ww, &hh);
   if (!dst) return;

   if ((ob->info->rotation == 0) || (ob->info->rotation == 180))
     {
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 0, 0, ww, hh);
        if (ob->info->rotation == 0)
          dst += (bpl * rect.y) + (rect.x * bpp);
        else if (ob->info->rotation == 180)
          {
             pr = rect;
             rx = pr.w - rect.w;
             ry = pr.h - rect.h;
             src += (update->cache_entry.w * ry) + rx;
          }
        w -= rx;
     }
   else
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 0, 0, ww, hh);
        rx = pr.w - rect.w;
        ry = pr.h - rect.h;
        if (ob->info->rotation == 90)
          src += ry;
        else if (ob->info->rotation == 270)
          src += (update->cache_entry.w * rx);
        w -= ry;
     }

   if ((rect.w <= 0) || (rect.h <= 0)) return;

   wid = bpl / bpp;
   dst += (bpl * rect.y) + (rect.x * bpp);
   conv_func(src, dst, update->cache_entry.w - w, wid - rect.w,
             rect.w, rect.h, x + rx, y + ry, NULL);
}

void
_outbuf_idle_flush(Outbuf *ob EINA_UNUSED)
{
   /* _outbuf_update_regions_remove(ob); */
   /* TODO: release FBs */
}

void
_outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage EINA_UNUSED, Tilebuf_Rect *buffer_damage EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Eina_Rectangle *rects;
   RGBA_Image *img;
   int update_count = 0;
   unsigned int i = 0;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (!ob->update_regions)
     {
        Eina_Rectangle *rect;
        Eina_Array_Iterator itr;

        /* get count of updates */
        update_count = eina_array_count_get(&ob->onebuf_regions);
        if (update_count == 0) return;

        /* allocate space for rectangles */
        rects = alloca(update_count * sizeof(Eina_Rectangle));
        if (!rects) return;

        EINA_ARRAY_ITER_NEXT(&ob->onebuf_regions, i, rect, itr)
          {
             rects[i] = *rect;
             eina_rectangle_free(rect);
          }

        _drm_swapper_buffer_unmap(ob->swapper);
        _drm_swapper_swap(ob->swapper, rects, update_count);

        eina_array_clean(&ob->onebuf_regions);

        img = ob->onebuf;
        ob->onebuf = NULL;

        if (img) evas_cache_image_drop(&img->cache_entry);
     }
   else
     {
        /* get count of updates */
        update_count = eina_list_count(ob->update_regions);
        if (update_count == 0) return;

        /* allocate space for rectangles */
        rects = alloca(update_count * sizeof(Eina_Rectangle));
        if (!rects) return;

        /* loop updates */
        EINA_LIST_FREE(ob->update_regions, img)
          {
             Eina_Rectangle *rect;
             int x = 0, y = 0, w = 0, h = 0;

             rect = img->extended_info;
             if (!rect) continue;

             x = rect->x; y = rect->y; w = rect->w; h = rect->h;

             /* based on rotation, set rectangle position */
             if (ob->info->rotation == 0)
               {
                  rects[i].x = x;
                  rects[i].y = y;
               }
             else if (ob->info->rotation == 90)
               {
                  rects[i].x = y;
                  rects[i].y = (ob->w - x - w);
               }
             else if (ob->info->rotation == 180)
               {
                  rects[i].x = (ob->w - x - w);
                  rects[i].y = (ob->h - y - h);
               }
             else if (ob->info->rotation == 270)
               {
                  rects[i].x = (ob->h - y - h);
                  rects[i].y = x;
               }

             /* based on rotation, set rectangle size */
             if ((ob->info->rotation == 0) || (ob->info->rotation == 180))
               {
                  rects[i].w = w;
                  rects[i].h = h;
               }
             else if ((ob->info->rotation == 90) || (ob->info->rotation == 270))
               {
                  rects[i].w = h;
                  rects[i].h = w;
               }

             eina_rectangle_free(rect);
             evas_cache_image_drop(&img->cache_entry);
             i++;
          }

        _drm_swapper_buffer_unmap(ob->swapper);
        _drm_swapper_swap(ob->swapper, rects, update_count);
     }
}
