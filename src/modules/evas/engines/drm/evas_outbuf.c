#include "evas_engine.h"
#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

/* FIXME: We NEED to get the color map from the VT and use that for the mask */
#define RED_MASK 0xff0000
#define GREEN_MASK 0x00ff00
#define BLUE_MASK 0x0000ff

static void _outbuf_tick_schedule(int fd, void *data);

static Eina_Bool ticking = EINA_FALSE;

static void
_outbuf_tick_begin(void *data)
{
   Outbuf *ob;

   ob = data;
   ticking = EINA_TRUE;
   if (ob) _outbuf_tick_schedule(ob->fd, ob);
}

static void
_outbuf_tick_end(void *data EINA_UNUSED)
{
   ticking = EINA_FALSE;
}

static void
_outbuf_tick_source_set(Outbuf *ob)
{
   if (ob)
     {
        ecore_animator_custom_source_tick_begin_callback_set
          (_outbuf_tick_begin, ob);
        ecore_animator_custom_source_tick_end_callback_set
          (_outbuf_tick_end, ob);
        ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
     }
   else
     {
        ecore_animator_custom_source_tick_begin_callback_set(NULL, NULL);
        ecore_animator_custom_source_tick_end_callback_set(NULL, NULL);
        ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
     }
}

static void
_outbuf_tick_schedule(int fd EINA_UNUSED, void *data)
{
   Outbuf *ob;
   Outbuf_Fb *ofb;

   ob = data;
   if (!ticking) return;

   ofb = ob->priv.current;
   if (!ofb) return;

   drmVBlank vbl =
     {
        .request.type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT,
        .request.sequence = 1,
        .request.signal = (unsigned long)data,
     };

   if (drmWaitVBlank(fd, &vbl) < 0)
     _outbuf_tick_source_set(NULL);
}

static void
_cb_vblank(int fd, unsigned int frame EINA_UNUSED, unsigned int sec EINA_UNUSED, unsigned int usec EINA_UNUSED, void *data)
{
   ecore_animator_custom_tick();
   if (ticking) _outbuf_tick_schedule(fd, data);
}

static void
_cb_pageflip(int fd EINA_UNUSED, unsigned int frame EINA_UNUSED, unsigned int sec EINA_UNUSED, unsigned int usec EINA_UNUSED, void *data)
{
   Outbuf *ob;
   Outbuf_Fb *ofb;
   Ecore_Drm2_Fb *next;

   ob = data;

   ofb = ob->priv.current;
   if (ofb) ofb->busy = EINA_FALSE;

   next = ecore_drm2_output_next_fb_get(ob->priv.output);
   if (next)
     {
        ecore_drm2_output_next_fb_set(ob->priv.output, NULL);
        if (ecore_drm2_fb_flip(next, ob->priv.output, ob) < 0)
          _outbuf_tick_source_set(NULL);
     }
}

static Eina_Bool
_cb_drm_event(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Outbuf *ob;
   int ret;

   ob = data;
   ret = drmHandleEvent(ob->fd, &ob->ctx);
   if (ret)
     {
        WRN("drmHandleEvent failed to read an event");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_outbuf_buffer_swap(Outbuf *ob, Eina_Rectangle *rects, unsigned int count)
{
   /* Ecore_Drm2_Plane *plane; */
   Outbuf_Fb *ofb;

   ofb = ob->priv.current;
   if (!ofb) return;

   ecore_drm2_fb_dirty(ofb->fb, rects, count);
   if (ecore_drm2_fb_flip(ofb->fb, ob->priv.output, ob) < 0)
     {
        _outbuf_tick_source_set(NULL);
        return;
     }

   ofb->busy = EINA_TRUE;
   ofb->drawn = EINA_TRUE;
   ofb->age = 0;

   /* plane = ecore_drm2_plane_find(ob->priv.output, ofb->fb, ob->format); */
   /* if (plane) */
   /*   { */
   /*      drmVBlank vbl = */
   /*        { */
   /*           .request.type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT, */
   /*           .request.sequence = 1, */
   /*        }; */

   /*      vbl.request.type |= ecore_drm2_output_vblank_get(ob->priv.output); */
   /*      vbl.request.signal = (unsigned long)ofb; */

   /*      ecore_drm2_fb_dirty(ofb->fb, rects, count); */

   /*      if (!ecore_drm2_plane_fb_set(plane, ofb->fb)) */
   /*        { */
   /*           ERR("Failed to set FB on Plane"); */
   /*           return; */
   /*        } */

   /*      if (drmWaitVBlank(ob->fd, &vbl) < 0) */
   /*        { */
   /*           _outbuf_tick_source_set(NULL); */
   /*           return; */
   /*        } */

   /*      ofb->busy = EINA_TRUE; */
   /*      ofb->drawn = EINA_TRUE; */
   /*      ofb->age = 0; */

   /*      ob->priv.current = NULL; */
   /*   } */
   /* else */
   /*   WRN("Could not find a plane for this framebuffer"); */
}

static Eina_Bool
_outbuf_fb_create(Outbuf *ob, Outbuf_Fb *ofb)
{
   ofb->fb =
     ecore_drm2_fb_create(ob->fd, ob->w, ob->h,
                          ob->depth, ob->bpp, ob->format);
   if (!ofb->fb) return EINA_FALSE;

   ofb->age = 0;
   ofb->busy = EINA_FALSE;
   ofb->drawn = EINA_FALSE;
   ofb->valid = EINA_TRUE;

   return EINA_TRUE;
}

static void
_outbuf_fb_destroy(Outbuf_Fb *ofb)
{
   ecore_drm2_fb_destroy(ofb->fb);

   memset(ofb, 0, sizeof(*ofb));
   ofb->valid = EINA_FALSE;
   ofb->busy = EINA_FALSE;
   ofb->drawn = EINA_FALSE;
   ofb->age = 0;
}

Outbuf *
_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h)
{
   Outbuf *ob;
   char *num;
   int i = 0;

   ob = calloc(1, sizeof(Outbuf));
   if (!ob) return NULL;

   ob->w = w;
   ob->h = h;
   ob->fd = info->info.fd;
   ob->alpha = info->info.alpha;
   ob->rotation = info->info.rotation;

   ob->bpp = info->info.bpp;
   ob->depth = info->info.depth;
   ob->format = info->info.format;

   ob->priv.output = info->info.output;

   ob->priv.num = 3;

   num = getenv("EVAS_DRM_BUFFERS");
   if (num)
     {
        ob->priv.num = atoi(num);
        if (ob->priv.num <= 0) ob->priv.num = 3;
        else if (ob->priv.num > 4) ob->priv.num = 4;
     }

   for (i = 0; i < ob->priv.num; i++)
     {
        if (!_outbuf_fb_create(ob, &(ob->priv.ofb[i])))
          {
             WRN("Failed to create framebuffer %d", i);
             continue;
          }
     }

   /* setup vblank handler */
   memset(&ob->ctx, 0, sizeof(ob->ctx));
   ob->ctx.version = DRM_EVENT_CONTEXT_VERSION;
   ob->ctx.vblank_handler = _cb_vblank;
   ob->ctx.page_flip_handler = _cb_pageflip;

   ob->hdlr =
     ecore_main_fd_handler_add(ob->fd, ECORE_FD_READ, _cb_drm_event, ob,
                               NULL, NULL);

   _outbuf_tick_source_set(ob);

   return ob;
}

void
_outbuf_free(Outbuf *ob)
{
   int i = 0;

   for (i = 0; i < ob->priv.num; i++)
     _outbuf_fb_destroy(&ob->priv.ofb[i]);

   ecore_main_fd_handler_del(ob->hdlr);

   free(ob);
}

int
_outbuf_rotation_get(Outbuf *ob)
{
   return ob->rotation;
}

void
_outbuf_reconfigure(Outbuf *ob, int w, int h, int rotation, Outbuf_Depth depth)
{
   int i = 0;
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
        format = DRM_FORMAT_RGBX8888;
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
        depth = ob->depth;
        format = ob->format;
        break;
     }

   if ((ob->w == w) && (ob->h == h) && (ob->rotation == rotation) &&
       (ob->depth == depth) && (ob->format == format))
     return;

   ob->depth = depth;
   ob->format = format;
   ob->rotation = rotation;

   ob->w = w;
   ob->h = h;
   if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        ob->w = h;
        ob->h = w;
     }

   for (i = 0; i < ob->priv.num; i++)
     _outbuf_fb_destroy(&ob->priv.ofb[i]);

   for (i = 0; i < ob->priv.num; i++)
     {
        if (!_outbuf_fb_create(ob, &(ob->priv.ofb[i])))
          {
             WRN("Failed to create framebuffer %d", i);
             continue;
          }
     }
}

static Outbuf_Fb *
_outbuf_fb_wait(Outbuf *ob)
{
   int iter = 0, i = 0;

   while (iter++ < 10)
     {
        for (i = 0; i < ob->priv.num; i++)
          {
             if (ob->priv.ofb[i].busy) continue;
             if (ob->priv.ofb[i].valid) return &(ob->priv.ofb[i]);
          }

        drmHandleEvent(ob->fd, &ob->ctx);
     }

   return NULL;
}

static Eina_Bool
_outbuf_fb_assign(Outbuf *ob)
{
   int i;

   ob->priv.current = _outbuf_fb_wait(ob);

   if (!ob->priv.current)
     {
        WRN("No Free Buffers. Dropping a frame");
        for (i = 0; i < ob->priv.num; i++)
          {
             if (ob->priv.ofb[i].valid)
               {
                  ob->priv.ofb[i].age = 0;
                  ob->priv.ofb[i].drawn = EINA_FALSE;
               }
          }

        return EINA_FALSE;
     }

   for (i = 0; i < ob->priv.num; i++)
     {
        if ((ob->priv.ofb[i].valid) && (ob->priv.ofb[i].drawn))
          {
             ob->priv.ofb[i].age++;
             if (ob->priv.ofb[i].age > ob->priv.num)
               {
                  ob->priv.ofb[i].age = 0;
                  ob->priv.ofb[i].drawn = EINA_FALSE;
               }
          }
     }

   return EINA_TRUE;
}

Render_Engine_Swap_Mode
_outbuf_state_get(Outbuf *ob)
{
   int age;

   if (!_outbuf_fb_assign(ob)) return MODE_FULL;

   age = ob->priv.current->age;
   if (age > ob->priv.num) return MODE_FULL;
   else if (age == 1) return MODE_COPY;
   else if (age == 2) return MODE_DOUBLE;
   else if (age == 3) return MODE_TRIPLE;
   else if (age == 4) return MODE_QUADRUPLE;

   return MODE_FULL;
}

void *
_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *img = NULL;

   if ((w <= 0) || (h <= 0)) return NULL;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, ob->w, ob->h);

   if ((ob->rotation == 0))// && (ob->depth == 32))
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

        img->cache_entry.flags.alpha = ob->alpha;

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
        ob->priv.pending = 
          eina_list_append(ob->priv.pending, img);
     }

   return img;
}

void
_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert func = NULL;
   Eina_Rectangle rect = {0, 0, 0, 0}, pr;
   DATA32 *src;
   DATA8 *dst;
   Ecore_Drm2_Fb *buff;
   int bpp = 0, bpl = 0;
   int rx = 0, ry = 0;
   int bw = 0, bh = 0;

   /* check for valid output buffer */
   if (!ob) return;

   /* check for pending writes */
   if (!ob->priv.pending) return;

   /* check for valid source data */
   if (!(src = update->image.data)) return;

   /* check for valid desination data */
   if (!ob->priv.current) return;
   buff = ob->priv.current->fb;

   dst = ecore_drm2_fb_data_get(buff);
   if (!dst) return;

   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        func = 
          evas_common_convert_func_get(0, w, h, ob->bpp, 
                                       RED_MASK, GREEN_MASK, BLUE_MASK,
                                       PAL_MODE_NONE, ob->rotation);
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        func = 
          evas_common_convert_func_get(0, h, w, ob->bpp, 
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

   bpp = ob->bpp / 8;
   bw = ob->w;
   bh = ob->h;
   /* bpp = (ob->depth / 8); */
   /* if (bpp <= 0) return; */

   bpl = ecore_drm2_fb_stride_get(buff);

   if (ob->rotation == 0)
     {
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, bw, bh);
        dst += (bpl * rect.y) + (rect.x * bpp);
        w -= rx;
     }
   else if (ob->rotation == 180)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, bw, bh);
        rx = pr.w - rect.w;
        ry = pr.h - rect.h;
        src += (update->cache_entry.w * ry) + rx;
        w -= rx;
     }
   else if (ob->rotation == 90)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, bw, bh);
        rx = pr.w - rect.w; ry = pr.h - rect.h;
        src += ry;
        w -= ry;
     }
   else if (ob->rotation == 270)
     {
        pr = rect;
        RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.w, rect.h, 
                           0, 0, bw, bh);
        rx = pr.w - rect.w; ry = pr.h - rect.h;
        src += (update->cache_entry.w * rx);
        w -= ry;
     }

   if ((rect.w <= 0) || (rect.h <= 0)) return;

   func(src, dst, (update->cache_entry.w - w), ((bpl / bpp) - rect.w),
        rect.w, rect.h, x + rx, y + ry, NULL);
}

void
_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{

}

void
_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Eina_Rectangle *r;
   RGBA_Image *img;
   unsigned int n = 0, i = 0;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   /* get number of pending writes */
   n = eina_list_count(ob->priv.pending);
   if (n == 0) return;

   /* allocate rectangles */
   r = alloca(n * sizeof(Eina_Rectangle));
   if (!r) return;

   /* loop the pending writes */
   EINA_LIST_FREE(ob->priv.pending, img)
     {
        Eina_Rectangle *rect;
        int x = 0, y = 0, w = 0, h = 0;

        rect = img->extended_info;
        if (!rect) continue;

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
   _outbuf_buffer_swap(ob, r, n);
}
