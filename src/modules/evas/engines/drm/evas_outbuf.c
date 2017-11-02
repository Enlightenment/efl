#include "evas_engine.h"

/* FIXME: We NEED to get the color map from the VT and use that for the mask */
#define RED_MASK 0xff0000
#define GREEN_MASK 0x00ff00
#define BLUE_MASK 0x0000ff

static Eina_Bool
_outbuf_fb_create(Outbuf *ob, Outbuf_Fb *ofb, int w, int h)
{
   fprintf(stdout, "Evas Drm: Creating Framebuffer: %dx%d\n", w, h);

   ofb->fb =
     ecore_drm2_fb_create(ob->dev, w, h, ob->depth, ob->bpp, ob->format);
   if (!ofb->fb)
     {
        WRN("Failed To Create FB: %d %d", w, h);
        free(out);
        return NULL;
     }

   out->age = 0;
   out->drawn = EINA_FALSE;
   out->valid = EINA_TRUE;

   return out;
}

static void
_outbuf_fb_destroy(Outbuf_Fb *ofb)
{
   ecore_drm2_fb_discard(ofb->fb);

   memset(ofb, 0, sizeof(*ofb));
   ofb->valid = EINA_FALSE;
   ofb->drawn = EINA_FALSE;
   ofb->age = 0;
   free(ofb);
}

Outbuf *
_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h)
{
   Outbuf *ob;
   char *num;
   int i = 0, fw, fh;

   ob = calloc(1, sizeof(Outbuf));
   if (!ob) return NULL;

   ob->w = w;
   ob->h = h;
   ob->dev = info->info.dev;
   ob->alpha = info->info.alpha;

   ob->bpp = info->info.bpp;
   ob->depth = info->info.depth;
   ob->format = info->info.format;
   ob->rotation = info->info.rotation;
   ob->output = info->info.output;

   ob->num_buffers = 3;
   num = getenv("EVAS_DRM_BUFFERS");
   if (num)
     {
        ob->num_buffers = atoi(num);
        if (ob->num_buffers <= 0) ob->num_buffers = 3;
        else if (ob->num_buffers > 4) ob->num_buffers = 4;
     }

   if ((!w) || (!h)) return ob;

   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        fw = w;
        fh = h;
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        fw = h;
        fh = w;
     }

   for (i = 0; i < ob->num_buffers; i++)
     {
        if (!_outbuf_fb_create(ob, &(ob->ofb[i]), fw, fh))
          {
             WRN("Failed to create framebuffer %d", i);
             continue;
          }
     }

   return ob;
}

void
_outbuf_free(Outbuf *ob)
{
   Outbuf_Fb *ofb;

   while (ob->pending)
     {
        RGBA_Image *img;
        Eina_Rectangle *rect;

        img = ob->pending->data;
        ob->pending =
          eina_list_remove_list(ob->pending, ob->pending);

        rect = img->extended_info;

        evas_cache_image_drop(&img->cache_entry);

        eina_rectangle_free(rect);
     }

   _outbuf_flush(ob, NULL, NULL, EVAS_RENDER_MODE_UNDEF);
   /* TODO: idle flush */

   for (; i < ob->num_buffers; i++)
     _outbuf_fb_destroy(&ob->ofb[i]);

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
   int i = 0, fw, fh;
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

   ob->w = w;
   ob->h = h;
   ob->depth = depth;
   ob->format = format;
   ob->rotation = rotation;
   ob->priv.unused_duration = 0;

   for (; i < ob->num_buffers; i++)
     _outbuf_fb_destroy(&ob->ofb[i]);

   if ((!w) || (!h)) return;

   if ((ob->rotation == 0) || (ob->rotation == 180))
     {
        fw = w;
        fh = h;
     }
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     {
        fw = h;
        fh = w;
     }

   for (i = 0; i < ob->num_buffers; i++)
     {
        if (!_outbuf_fb_create(ob, &(ob->ofb[i]), fw, fh))
          {
             WRN("Failed to create framebuffer %d", i);
             continue;
          }
     }

   /* TODO: idle_flush */
}

static Outbuf_Fb *
_outbuf_fb_wait(Outbuf *ob)
{
   Eina_List *l;
   Outbuf_Fb *ofb, *best = NULL;
   int best_age = -1, num_required = 1, num_allocated = 0;

   fprintf(stderr, "FB Wait on Output %s\n",
           ecore_drm2_output_name_get(ob->output));

   /* We pick the oldest available buffer to avoid using the same two
    * repeatedly and then having the third be stale when we need it */
   for (i = 0; i < ob->num_buffers; i++)
     {
        if (ecore_drm2_fb_busy_get(ob->ofb[i].fb))
          {
             fprintf(stderr, "\t FB %d Busy\n", i);
             continue;
          }
        if (ob->ofb[i].valid && (ob->ofb[i].age > best_age))
          {
             best = i;
             best_age = ob->ofb[i].age;
          }
        if (ofb->valid && (ofb->age > best_age))
          {
             best = ofb;
             best_age = best->age;
          }
     }

   if (num_required < num_allocated)
      ob->priv.unused_duration++;
   else
      ob->priv.unused_duration = 0;

   /* If we've had unused buffers for longer than QUEUE_TRIM_DURATION, then
    * destroy the oldest buffer (currently in best) and recursively call
    * ourself to get the next oldest.
    */
   if (best && (ob->priv.unused_duration > QUEUE_TRIM_DURATION))
     {
        ob->priv.unused_duration = 0;
        ob->priv.fb_list = eina_list_remove(ob->priv.fb_list, best);
        _outbuf_fb_destroy(best);
        best = _outbuf_fb_wait(ob);
     }

   if (best >= 0) return &(ob->ofb[best]);
   return NULL;
}

static Eina_Bool
_outbuf_fb_assign(Outbuf *ob)
{
   int fw = 0, fh = 0;
   Outbuf_Fb *ofb;
   Eina_List *l;

   fprintf(stderr, "FB Assign on Output %s\n",
           ecore_drm2_output_name_get(ob->output));

   ob->draw = _outbuf_fb_wait(ob);
   while (!ob->draw)
     {
        ecore_drm2_fb_release(ob->output, EINA_TRUE);
        ob->draw = _outbuf_fb_wait(ob);
     }

   for (i = 0; i < ob->num_buffers; i++)
     {
        if ((ob->ofb[i].valid) && (ob->ofb[i].drawn))
          {
             ob->ofb[i].age++;
             if (ob->ofb[i].age > 4)
               {
                  ob->ofb[i].age = 0;
                  ob->ofb[i].drawn = EINA_FALSE;
               }
          }
     }

   return EINA_TRUE;
}

Render_Output_Swap_Mode
_outbuf_state_get(Outbuf *ob)
{
   int age = 0;

   if (!_outbuf_fb_assign(ob)) return MODE_FULL;

   if (ob->draw) age = ob->draw->age;

   if (age > ob->num_buffers) return MODE_FULL;
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
   Eina_Rectangle *rect;

   if ((w <= 0) || (h <= 0)) return NULL;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, ob->w, ob->h);

   if (!(rect = eina_rectangle_new(x, y, w, h)))
     return NULL;

   img = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());

   if (!img)
     {
        eina_rectangle_free(rect);
        return NULL;
     }

   img->cache_entry.flags.alpha = ob->alpha;

   evas_cache_image_surface_alloc(&img->cache_entry, w, h);

   img->extended_info = rect;

   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = w;
   if (ch) *ch = h;

   /* add this cached image data to pending writes */
   ob->pending =
     eina_list_append(ob->pending, img);

   return img;
}

void
_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert func = NULL;
   Eina_Rectangle rect = {0, 0, 0, 0}, pr;
   DATA32 *src;
   DATA8 *dst;
   int bpp = 0, bpl = 0;
   int rx = 0, ry = 0;
   int bw = 0, bh = 0;

   /* check for valid output buffer */
   if (!ob) return;

   /* check for pending writes */
   if (!ob->pending) return;

   /* check for valid source data */
   if (!(src = update->image.data)) return;

   /* check for valid desination data */
   if (!ob->draw) return;

   dst = ecore_drm2_fb_data_get(ob->draw->fb);
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

   bpl = ecore_drm2_fb_stride_get(ob->draw->fb);

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
_outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage EINA_UNUSED, Tilebuf_Rect *buffer_damage EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Eina_Rectangle *r;
   RGBA_Image *img;
   unsigned int i = 0;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (ob->count) free(ob->rects);

   /* get number of pending writes */
   ob->count = eina_list_count(ob->pending);
   if (ob->count == 0) return;

   /* allocate rectangles */
   ob->rects = malloc(ob->count * sizeof(Eina_Rectangle));
   if (!ob->rects) return;

   r = ob->rects;

   /* loop the pending writes */
   EINA_LIST_FREE(ob->pending, img)
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

        evas_cache_image_drop(&img->cache_entry);

        i++;
     }
}

static void
_outbuf_buffer_swap(Outbuf *ob, Eina_Rectangle *rects, unsigned int count)
{
   Outbuf_Fb *ofb;

   ofb = ob->draw;
   if (!ofb) return;

   ecore_drm2_fb_dirty(ofb->fb, rects, count);

   if (!ob->plane)
     ob->plane = ecore_drm2_plane_assign(ob->output, ofb->fb, 0, 0);
   else
     ecore_drm2_plane_fb_set(ob->plane, ofb->fb);

   ecore_drm2_fb_flip(ofb->fb, ob->output);
   ofb->drawn = EINA_TRUE;
   ofb->age = 0;
}

void
_outbuf_redraws_clear(Outbuf *ob)
{
   if (!ob->count) return;

   _outbuf_buffer_swap(ob, ob->rects, ob->count);
   free(ob->rects);
   ob->count = 0;
}
