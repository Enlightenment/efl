#include "evas_engine.h"
#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

/* FIXME: We NEED to get the color map from the VT and use that for the mask */
#define RED_MASK 0xff0000
#define GREEN_MASK 0x00ff00
#define BLUE_MASK 0x0000ff

static Eina_Bool 
_evas_outbuf_buffer_new(Outbuf *ob, Buffer *buff)
{
   buff->w = ob->w;
   buff->h = ob->h;
   if (buff->w < ob->priv.mode.hdisplay) buff->w = ob->priv.mode.hdisplay;
   if (buff->h < ob->priv.mode.vdisplay) buff->h = ob->priv.mode.vdisplay;

   /* create a dumb framebuffer */
   if (!evas_drm_framebuffer_create(ob->priv.fd, buff, ob->depth))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void 
_evas_outbuf_buffer_put(Outbuf *ob, Buffer *buffer, Eina_Rectangle *rects, unsigned int count)
{
   /* validate input params */
   if ((!ob) || (!buffer)) return;

#ifdef DRM_MODE_FEATURE_DIRTYFB
   drmModeClip *clip;
   unsigned int i = 0;
   int ret;

   clip = alloca(count * sizeof(drmModeClip));
   for (i = 0; i < count; i++)
     {
        clip[i].x1 = rects[i].x;
        clip[i].y1 = rects[i].y;
        clip[i].x2 = rects[i].w;
        clip[i].y2 = rects[i].h;
     }

   /* DBG("Marking FB Dirty: %d", buffer->fb); */
   ret = drmModeDirtyFB(ob->priv.fd, buffer->fb, clip, count);
   if (ret)
     {
        if (ret == -EINVAL)
          ERR("Could not set FB Dirty: %m");
     }
#endif
}

static void 
_evas_outbuf_buffer_swap(Outbuf *ob, Eina_Rectangle *rects, unsigned int count)
{
   Buffer *buff;

   buff = &(ob->priv.buffer[ob->priv.curr]);

   /* if this buffer is not valid, we need to set it */
   if (!buff->valid) evas_drm_outbuf_framebuffer_set(ob, buff);

   /* mark the fb as dirty */
   _evas_outbuf_buffer_put(ob, buff, rects, count);

   /* send this buffer to the crtc */
   evas_drm_framebuffer_send(ob, buff);
}

Outbuf *
evas_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h)
{
   Outbuf *ob;
   char *num;
   int i = 0;

   /* try to allocate space for outbuf */
   if (!(ob = calloc(1, sizeof(Outbuf)))) return NULL;

   /* set properties of outbuf */
   ob->w = w;
   ob->h = h;
   if (ob->w < ob->priv.mode.hdisplay) ob->w = ob->priv.mode.hdisplay;
   if (ob->h < ob->priv.mode.vdisplay) ob->h = ob->priv.mode.vdisplay;

   ob->depth = info->info.depth;
   ob->rotation = info->info.rotation;
   ob->destination_alpha = info->info.destination_alpha;
   ob->vsync = info->info.vsync;

   /* set drm card fd */
   ob->priv.fd = info->info.fd;

   /* try to setup the drm card for this outbuf */
   if (!evas_drm_outbuf_setup(ob))
     {
        ERR("Could not setup output");
        free(ob);
        return NULL;
     }

   info->info.output = ob->priv.fb;

   ob->priv.num = NUM_BUFFERS;

   /* check for buffer override */
   if ((num = getenv("EVAS_DRM_BUFFERS")))
     {
        ob->priv.num = atoi(num);

        /* cap maximum # of buffers */
        if (ob->priv.num <= 0) ob->priv.num = 1;
        else if (ob->priv.num > 3) ob->priv.num = 3;
     }

   /* check for vsync override */
   if ((num = getenv("EVAS_DRM_VSYNC")))
     ob->vsync = atoi(num);

   /* try to create buffers */
   for (; i < ob->priv.num; i++)
     {
        if (!_evas_outbuf_buffer_new(ob, &(ob->priv.buffer[i])))
          break;
     }

   /* set the front buffer to be the one on the crtc */
   evas_drm_outbuf_framebuffer_set(ob, &(ob->priv.buffer[0]));

   /* set back buffer as first one to draw into */
   /* ob->priv.curr = (ob->priv.num - 1); */

   ob->info = info;

   return ob;
}

void
evas_outbuf_free(Outbuf *ob)
{
   int i = 0;

   /* destroy the old buffers */
   for (; i < ob->priv.num; i++)
     evas_drm_framebuffer_destroy(ob->priv.fd, &(ob->priv.buffer[i]));

   /* free allocate space for outbuf */
   free(ob);
}

void
evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth)
{
   int i = 0;

   /* check for changes */
   if ((ob->w == w) && (ob->h == h) &&
       (ob->destination_alpha == ob->info->info.destination_alpha) &&
       (ob->rotation == rot) &&
       (ob->depth == depth))
     return;

   /* set new outbuf properties */
   ob->rotation = rot;
   ob->depth = depth;
   ob->destination_alpha = ob->info->info.destination_alpha;

   /* handle rotation */
   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        ob->w = w;
        ob->h = h;
        if (ob->w < ob->priv.mode.hdisplay) ob->w = ob->priv.mode.hdisplay;
        if (ob->h < ob->priv.mode.vdisplay) ob->h = ob->priv.mode.vdisplay;
     }
   else
     {
        ob->w = h;
        ob->h = w;
        if (ob->w < ob->priv.mode.vdisplay) ob->w = ob->priv.mode.vdisplay;
        if (ob->h < ob->priv.mode.hdisplay) ob->h = ob->priv.mode.hdisplay;
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

Render_Engine_Swap_Mode
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

void *
evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *img = NULL;

   if ((w <= 0) || (h <= 0)) return NULL;

   /* DBG("Outbuf Region New: %d %d %d %d", x, y, w, h); */

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, ob->w, ob->h);

   if ((ob->rotation == 0) && (ob->depth == 32))
     {
        Eina_Rectangle *rect;

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

#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_surface_alloc(&img->cache_entry, w, h);
        else
#endif
          evas_cache_image_surface_alloc(&img->cache_entry, w, h);

        img->extended_info = rect;

        if (cx) *cx = 0;
        if (cy) *cy = 0;
        if (cw) *cw = w;
        if (ch) *ch = h;

        /* add this cached image data to pending writes */
        ob->priv.pending_writes = 
          eina_list_append(ob->priv.pending_writes, img);
     }

   return img;
}

void 
evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert func = NULL;
   Eina_Rectangle rect = {0, 0, 0, 0}, pr;
   DATA32 *src;
   DATA8 *dst;
   Buffer *buff;
   int bpp = 0, bpl = 0;
   int rx = 0, ry = 0;

   /* check for valid output buffer */
   if (!ob) return;

   /* check for pending writes */
   if (!ob->priv.pending_writes) return;

   /* check for valid source data */
   if (!(src = update->image.data)) return;

   /* check for valid desination data */
   buff = &(ob->priv.buffer[ob->priv.curr]);
   if (!(dst = buff->data)) return;

   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        func = 
          evas_common_convert_func_get(0, w, h, ob->depth, 
                                       RED_MASK, GREEN_MASK, BLUE_MASK,
                                       PAL_MODE_NONE, ob->rotation);
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        func = 
          evas_common_convert_func_get(0, h, w, ob->depth, 
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

   bpp = (ob->depth / 8);
   if (bpp <= 0) return;

   bpl = buff->stride;

   if (ob->rotation == 0)
     {
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, buff->w, buff->h);
        dst += (bpl * rect.y) + (rect.x * bpp);
        w -= rx;
     }
   else if (ob->rotation == 180)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, buff->w, buff->h);
        rx = pr.w - rect.w;
        ry = pr.h - rect.h;
        src += (update->cache_entry.w * ry) + rx;
        w -= rx;
     }
   else if (ob->rotation == 90)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, buff->w, buff->h);
        rx = pr.w - rect.w; ry = pr.h - rect.h;
        src += ry;
        w -= ry;
     }
   else if (ob->rotation == 270)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, buff->w, buff->h);
        rx = pr.w - rect.w; ry = pr.h - rect.h;
        src += (update->cache_entry.w * rx);
        w -= ry;
     }

   if ((rect.w <= 0) || (rect.h <= 0)) return;

   func(src, dst, (update->cache_entry.w - w), ((bpl / bpp) - rect.w),
        rect.w, rect.h, x + rx, y + ry, NULL);
}

void
evas_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{
   /* evas_cache_image_drop(&update->cache_entry); */
}

void
evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Eina_Rectangle *r;
   RGBA_Image *img;
   unsigned int n = 0, i = 0;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   /* get number of pending writes */
   n = eina_list_count(ob->priv.pending_writes);
   if (n == 0) return;

   /* allocate rectangles */
   if (!(r = alloca(n * sizeof(Eina_Rectangle)))) return;

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
             r[i].x = x;
             r[i].y = y;
          }
        else if (ob->rotation == 90)
          {
             r[i].x = y;
             r[i].y = (ob->w - x - w);
          }
        else if (ob->rotation == 180)
          {
             r[i].x = (ob->w - x - w);
             r[i].y = (ob->h - y - h);
          }
        else if (ob->rotation == 270)
          {
             r[i].x = (ob->h - y - h);
             r[i].y = x;
          }

        /* based on rotation, set rectangle size */
        if ((ob->rotation == 0) || (ob->rotation == 180))
          {
             r[i].w = w;
             r[i].h = h;
          }
        else if ((ob->rotation == 90) || (ob->rotation == 270))
          {
             r[i].w = h;
             r[i].h = w;
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
   _evas_outbuf_buffer_swap(ob, r, n);
}

int
evas_outbuf_get_rot(Outbuf *ob)
{
   return ob->info->info.rotation;
}
